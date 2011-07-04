<?php

function campaignmonitor_admin_settings_lists() {
  $form = array('#tree' => TRUE);

  $account = variable_get('campaignmonitor_account', FALSE);
  if (!$account) {
    drupal_set_message(t('You have not entered your account information yet, henc lists from Campaign Monitor can not be downloaded.'), 'error');
    return;
  }

  // Download list information form campaign monitor
  $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);
  $lists = $cm->getLists();

  $header = array(
    array('data' => 'title', 'field' => 'title', 'sort' => 'asc'),
    array('data' => 'List ID', 'field' => 'id'),    
    array('data' => 'Status', 'field' => 'status'),
    array('data' => 'Edit', 'field' => 'edit'),
  );

  $rows = array();
  foreach ($lists as $id => $list) {
    $rows[] = array(
      'data' => array(
          $list['name'],
          $id,
          'Unknown',
          l(t('Edit'), 'admin/config/services/campaignmonitor/lists/' . $id . '/edit'),
        ),
    );
  }

  $html = theme('table',
    array(
	    'header' => $header,
	    'rows'=>$rows,
	    'sticky' => TRUE,
	    'empty' => 'There where no lists found...',
	  )
	);

	//Append pager
	$html .= theme('pager', array('tags' => array()));

	return ($html);
}

function campaignmonitor_admin_settings_list_edit($form, &$form_state, $listId) {
  $form =  array('#tree' => TRUE);

  // Get campaign monitor connection.
  $account = variable_get('campaignmonitor_account', array());
  $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);

  // Load extended information about the list.
  $list = $cm->getExtendedList($listId);
  
  // Add list id to the form.
  $form['listId'] = array(
    '#type' => 'hidden',
    '#value' => $listId,
  );

  // Set this form name (index).
  $form_key = 'campaignmonitor_list_' . $listId;

  // Get previouse save list information.
  $defaults = variable_get($form_key, array());
  
  $form[$form_key]['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('List options'),
    '#description' => t('Changing the values will result in an updated of the values on the Campaign Monitor homepage.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form[$form_key]['options']['listname'] = array(
    '#type' => 'textfield',
    '#title' => t('List name'),
    '#default_value' => $list['name'],
    '#required' => TRUE,
  );

  $form[$form_key]['options']['UnsubscribePage'] = array(
    '#type' => 'textfield',
    '#title' => t('Unsubscribe page'),
    '#default_value' => $list['details']['UnsubscribePage'],
  );

  $form[$form_key]['options']['ConfirmationSuccessPage'] = array(
    '#type' => 'textfield',
    '#title' => t('Confirmation success page'),
    '#default_value' => $list['details']['ConfirmationSuccessPage'],
  );

  $form[$form_key]['options']['ConfirmedOptIn'] = array(
    '#type' => 'checkbox',
    '#title' => t('Confirmed Opt In'),
    '#options' => array(1),
    '#default_value' => $list['details']['ConfirmedOptIn'],
  );
  
  $form[$form_key]['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form[$form_key]['display']['name'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display Name field'),
    '#description' => t('When subscribing, should the name field be displayed.'),
    '#default_value' => isset($defaults['display']['name']) ? $defaults['display']['name'] : 0,
  );

  // List costom fields
  if (!empty($list['CustomFields'])) {
      $options = array();
      foreach ($list['CustomFields'] as $key => $value) {
        $options[$key] = $value['FieldName'];
      }


    $form[$form_key]['CustomFields'] = array(
      '#type' => 'fieldset',
      '#title' => t('Custom fields'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    $form[$form_key]['CustomFields']['selected'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Avaliable fields'),
      '#description' => t('Select the fields that should be displayed on subscription forms.'),
      '#options' => $options,
      '#default_value' => isset($defaults['CustomFields']['selected']) ? $defaults['CustomFields']['selected'] : array(),
    );
  }
  
  // Give the form system feed and look.
  $form = system_settings_form($form);

  // Override system form submit.
  $form['#submit'] = array('campaignmonitor_admin_settings_list_edit_submit');

  return $form;
}

function campaignmonitor_admin_settings_list_edit_submit($form, &$form_state) {

  // Build array with basic information.
  $values = $form_state['values']['campaignmonitor_list_' . $form_state['values']['listId']];
  $options = array(
    'Title' => check_plain($values['options']['listname']),
    'UnsubscribePage' => check_plain($values['options']['UnsubscribePage']),
    'ConfirmedOptIn' => $values['options']['ConfirmedOptIn'] ? TRUE : FALSE,
    'ConfirmationSuccessPage' => check_plain($values['options']['ConfirmationSuccessPage']),
  );

  // Get connected.
  $account = variable_get('campaignmonitor_account', array());
  $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);

  // Update the information.
  if (!$cm->updateList($form_state['values']['listId'], $options)) {
    drupal_set_message(t('The list was not updated correctly at Campaign Monitor.'), 'error');
  }

  // Remove list options.
  unset($form_state['values']['campaignmonitor_list_' . $form_state['values']['listId']]['options']);

  // Save display options and custom field selection
  system_settings_form_submit($form, $form_state);
}