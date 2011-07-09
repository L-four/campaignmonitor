<?php

function campaignmonitor_admin_settings_lists() {
  $account = variable_get('campaignmonitor_account', FALSE);
  if (!$account) {
    drupal_set_message(t('You have not entered your account information yet, henc lists from Campaign Monitor can not be downloaded.'), 'error');
    return;
  }

  // Download list information form campaign monitor
  $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);
  $lists = $cm->getLists();

  $header = array(
    array('data' => t('Title'), 'field' => 'title', 'sort' => 'asc'),
    array('data' => t('List ID'), 'field' => 'id'),
    array('data' => t('Subscribed / Unsubscribed'), 'field' => 'status'),
    array('data' => t('Operations'), 'field' => 'Operations'),
  );

  $rows = array();
  foreach ($lists as $id => $list) {
    // Define supported oprations and allow other modules to add more.
    $operations = array(
      'Edit' => l(t('Edit'), 'admin/config/services/campaignmonitor/lists/' . $id . '/edit'),
      'Delete' => l(t('Delete'), 'admin/config/services/campaignmonitor/lists/' . $id . '/delete')
    );
    drupal_alter('campaignmonitor_operations', $operations);
    
    $stats = $cm->getListStats($id);
    $rows[] = array(
      'data' => array(
          $list['name'],
          $id,
          $stats['TotalActiveSubscribers'] . ' / ' . $stats['TotalUnsubscribes'],
          implode(' ', $operations),
        ),
    );
  }

  $html = theme('table',
    array(
	    'header' => $header,
	    'rows' => $rows,
	    'sticky' => TRUE,
	    'empty' => 'Lists not found or non created yet...',
	  )
	);

	//Append pager
	$html .= theme('pager', array('tags' => array()));

  // Give it some styling.
  drupal_add_css(drupal_get_path('module', 'campaignmonitor') . '/css/campaignmonitor.admin.css');

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
    //'#options' => array('1'),
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

  // Add validation function.
  $form['#validate'][] = 'campaignmonitor_admin_settings_list_edit_validate';
  return $form;
}

function campaignmonitor_admin_settings_list_edit_validate($form, &$form_state) {
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
    $error = $cm->getLatestError();
    form_set_error('', $error['message']);
    drupal_set_message(t('The list options was not updated correctly at Campaign Monitor.'), 'error');
    return FALSE;
  }

  // Remove list options.
  unset($form_state['values']['campaignmonitor_list_' . $form_state['values']['listId']]['options']);

  // Save display options and custom field selection
  system_settings_form_submit($form, $form_state);
}

function campaignmonitor_admin_settings_list_delete($form, &$form_state, $list_id) {
  // Get campaign monitor connection.
  $account = variable_get('campaignmonitor_account', array());
  $lists = CampaignMonitor::getConnector($account['api_key'], $account['client_id'])->getLists();

  $str = t('You are about to delete the list "@list", which may still have users
            subscribed to it.<br /><br />This action cannot be undone.',
          array('@list' => $lists[$list_id]['name']));

  // Build confirmation form.
  $form['list_id'] = array('#type' => 'hidden', '#value' => $list_id);
  $form['#submit'][] = 'campaignmonitor_admin_settings_list_delete_submit';
  $confirm_question = 'Are you sure you want to delete this list ?';
  return confirm_form($form,
                    $confirm_question,
                    'admin/config/services/campaignmonitor/lists', $str,
                    t('Delete'), t('Cancel'));
}

function campaignmonitor_admin_settings_list_delete_submit($form, &$form_state) {
  // Delete list at campaign monitor.
  $account = variable_get('campaignmonitor_account', array());
  $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);

  // Delete the list at campaign monitor.
  if ($cm->deleteList($form_state['values']['list_id'])) {
    drupal_set_message(t('The list have been deleted.'), 'status');
  }
  else {
    drupal_set_message(t('The list could not be deleted.'), 'error');
  }

  // Clear blocks cache.
  _block_rehash();

  // Set message and redirect to list overview.
  $form_state['redirect'] = 'admin/config/services/campaignmonitor/lists';
}


function campaignmonitor_admin_settings_list_create_form() {
  $form = array();

  $form['listname'] = array(
    '#type' => 'textfield',
    '#title' => t('List name'),
    '#default_value' => '',
    '#required' => TRUE,
  );

  $form['UnsubscribePage'] = array(
    '#type' => 'textfield',
    '#title' => t('Unsubscribe page'),
    '#default_value' => '',
  );

  $form['ConfirmationSuccessPage'] = array(
    '#type' => 'textfield',
    '#title' => t('Confirmation success page'),
    '#default_value' => '',
  );

  $form['ConfirmedOptIn'] = array(
    '#type' => 'checkbox',
    '#title' => t('Confirmed Opt In'),
    '#default_value' => false,
  );

  $form = system_settings_form($form);
  $form['#submit'] = array('campaignmonitor_admin_settings_list_create_form_submit');
  $form['#validate'][] = 'campaignmonitor_admin_settings_list_create_form_validate';

  return $form;
}

function campaignmonitor_admin_settings_list_create_form_validate($form, &$form_state) {
  $account = variable_get('campaignmonitor_account', array());
  $cm = CampaignMonitor::getConnector($account['api_key'], $account['client_id']);
  $result = $cm->createList($account['client_id'],
                            $form_state['values']['listname'],
                            $form_state['values']['UnsubscribePage'],
                            $form_state['values']['ConfirmedOptIn'],
                            $form_state['values']['ConfirmationSuccessPage']);
  if (!$result) {
    $error = $cm->getLatestError();
    form_set_error('listname', $error['message']);
    return FALSE;
  }
}

function campaignmonitor_admin_settings_list_create_form_submit($form, &$form_state) {
  drupal_set_message('List has been created at Campaign monitor.', 'status');  
}