<?php

/*
 * @file
 * Implementation of the administration interface for the module.
 */

/**
 * Menu callback that creates the administartion settings form.
 *
 * @return array $form
 */
function campaignmonitor_admin_settings_form() {
  $api_key = variable_get(CM_API_KEY, FALSE);
  $client_id = variable_get(CM_CLIENT_ID, '');

  $form['campaignmonitor_account_details'] = array(
    '#type' => 'fieldset',
    '#collapsible' => $api_key ? TRUE : FALSE,
    '#collapsed' => $api_key ? TRUE : FALSE,
    '#title' => t('Account Details'),
  );

  $form['campaignmonitor_account_details'][CM_API_KEY] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#default_value' => $api_key,
    '#required' => TRUE,
    '#size' => 50,
    '#maxlength' => 200,
    '#description' => t('Your Campaign Monitor API Key. See <a href="http://www.campaignmonitor.com/api/required/">documentation</a>.')
  );

  $form['campaignmonitor_account_details'][CM_CLIENT_ID] = array(
    '#type' => 'textfield',
    '#title' => t('Client ID'),
    '#default_value' => $client_id,
    '#required' => TRUE,
    '#size' => 50,
    '#maxlength' => 200,
    '#description' => t('Your Campaign Monitor Client ID. See <a href="http://www.campaignmonitor.com/api/required/">documentation</a>.')
  );

  // List options for the lists featched from the campaign monintor account, If
  // API key is defined.
  if ($api_key) {
    $form['campaignmonitor_general_options'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('General Options'),
    );

    $form['campaignmonitor_general_options'][CM_DISPLAY_ON] = array(
      '#type' => 'checkboxes',
      '#title' => t('Display Options'),
      '#default_value' => variable_get(CM_DISPLAY_ON, array()),
      '#options' => array(
        'contact' => t('Contact Page'),
        'registration' => t('Registration Page'),
        'userpage' => t('User Page'),
      ),
      '#description' => t('Choose which forms you want to display the Join Newsletter checkbox(es).')
    );

    $form['campaignmonitor_general_options'][CM_CHECKBOX_DISPLAY_TEXT] = array(
      '#type' => 'textfield',
      '#title' => t('Display Text for Checkbox'),
      '#default_value' => variable_get(CM_CHECKBOX_DISPLAY_TEXT, CM_CHECKBOX_DISPLAY_TEXT_DEFAULT),
      '#size' => 50,
      '#maxlength' => 50,
      '#description' => t("This text will display next to the checkbox on the selected forms.")
    );

    $form['campaignmonitor_general_options'][CM_USERPAGE_DISPLAY_TEXT] = array(
      '#type' => 'textfield',
      '#title' => t('Display Text for User Page'),
      '#default_value' => variable_get(CM_USERPAGE_DISPLAY_TEXT, CM_USERPAGE_DISPLAY_TEXT_DEFAULT),
      '#size' => 50,
      '#maxlength' => 50,
      '#description' => t("This text will display next to the checkbox on the user profile page.")
    );

    $form['campaignmonitor_general_options'][CM_PAST_CAMPAIGN_URL] = array(
      '#type' => 'textfield',
      '#title' => t('Past Campaign URL'),
      '#default_value' => variable_get(CM_PAST_CAMPAIGN_URL, ''),
      '#size' => 100,
      '#maxlength' => 100,
      '#description' => t("This is required if you want to use the page that displays past campaigns. You can find this value if you go to Manage Clients, click on the client, go to the link that tells you how to display past campaigns, then copy the URL ONLY from the html given. The URL is in between the src=\"\" value.")
    );

    $form['campaignmonitor_general_options'][CM_CONNECTION_TIMEOUT] = array(
      '#type' => 'textfield',
      '#title' => t('Connection timeout'),
      '#default_value' => variable_get(CM_CONNECTION_TIMEOUT, CM_CONNECTION_TIMEOUT_DEFAULT),
      '#size' => 10,
      '#maxlength' => 10,
      '#description' => t("If your server can't get through to the API, or the API server is down, this is the amount of time until the connection times out in seconds. Default is %default seconds.", array('%default' => CM_CONNECTION_TIMEOUT_DEFAULT))
    );

    // Load lists form campaign monitor.
    $available_lists = _campaignmonitor_get_lists($api_key, $client_id);

    $form[CM_LISTS] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#title' => t('Available Lists'),
      '#description' => t('Configure the behaviour of the individuale lists avaliable.'),
      '#tree' => TRUE,
    );

    // Display message, if lists not are avaliable.
    if (!count($available_lists)) {
      unset($form[CM_LISTS]['#description']);
      $form[CM_LISTS]['empty'] = array(
        '#markup' => t('You Campaign Monitor account do not have any lists avaliable.'),
      );
    }

    // Get saved lists configuration.
    $saved_lists = variable_get(CM_LISTS, array());
    foreach ($available_lists as $list_id => $list_name) {
      $saved_list = $saved_lists[$list_id];

      // Get details about the current list form campaign monitor.
      $list_detail = _campaignmonitor_get_list_detail($api_key, $list_id);
      $list_custom_fields = _campaignmonitor_get_custom_fields($api_key, $list_id);

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id] = array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => $list_name,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['info'] = array(
        '#type' => 'fieldset',
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#title' => t('List information'),
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['info']['list_id'] = array(
        '#type' => 'value',
        '#value' => $list_id,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['info']['name'] = array(
        '#type' => 'value',
        '#value' => $list_name,
      );

      // Build string with list details.
      $details = '<p>' . t('Confirm Opt in: %optin<br/>Unsubscribe Page: %unsub<br/>Confirmation Success Page: %conf',
                    array('%optin' => isset($list_detail['ConfirmOptIn']) ? $list_detail['ConfirmOptIn'] : '',
                          '%unsub' => isset($list_detail['UnsubscribePage']) ? $list_detail['UnsubscribePage'] : '',
                          '%conf'  => isset($list_detail['ConfirmationSuccessPage']) ? $list_detail['ConfirmationSuccessPage'] : '')
                  ) . '</p>';
      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['info']['list_detail'] = array(
        '#markup' => $details,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['oncontactpage'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include on Contact page'),
        '#description' => t('If displaying on the contact page is enabled in general options, checking this includes this list in the display.'),
        '#default_value' => $saved_list->oncontactpage,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['onregopage'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include on Registration page'),
        '#description' => t('If displaying on the registration page is enabled in general options, checking this includes this list in the display.'),
        '#default_value' => $saved_list->onregopage,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['onuserpage'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include on User page'),
        '#description' => t('If displaying on the user page is enabled in general options, checking this includes this list in the display.'),
        '#default_value' => $saved_list->onuserpage,
      );

      $form[CM_LISTS]['campaignmonitor_list_'. $list_id]['displayname'] = array(
        '#type' => 'checkbox',
        '#title' => t('Display Name field'),
        '#description' => t('When subscribing, should the name field be displayed.'),
        '#default_value' => $saved_list->displayname,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['namekey'] = array(
        '#type' => 'select',
        '#title' => t('Name'),
        '#options' => campaignmonitor_get_field_keys(),
        '#default_value' => $saved_list->namekey,
        '#description' => t('Use tokens to automatically fill in the name field.')
      );

      if (count($list_custom_fields) > 0) {
        $form[CM_LISTS]['campaignmonitor_list_' . $list_id][CM_LISTS_CF] = array(
          '#type' => 'fieldset',
          '#title' => t('Custom Fields'),
          '#description' => t('Select which custom fields to use (you can define these at your Campaign Monitor account).'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        );

        // Get previous saved information about custom fields.
        $saved_cfs = property_exists($saved_list, 'customfields') ? $saved_list->customfields : array();
        foreach ($list_custom_fields as $cf) {
          $key = str_replace(array('[', ']'), '', $cf['Key']);
          if (!isset($saved_cfs[$key])) {
            $default_cfs = TRUE;
          }
          else {
            $default_cfs = $saved_cfs[$key]->display;
          }

          $form[CM_LISTS]['campaignmonitor_list_' . $list_id][CM_LISTS_CF]['custom_field_' . $key]['cf_id'] = array(
            '#type' => 'value',
            '#value' => $key,
          );

          $form[CM_LISTS]['campaignmonitor_list_' . $list_id][CM_LISTS_CF]['custom_field_' . $key]['display'] = array(
            '#type' => 'checkbox',
            '#title' => $cf['FieldName'],
            '#description' => t('Display this custom field in the subscribe forms.'),
            '#default_value' => $default_cfs,
          );
        }
      }
    }
  }

  $form['clear_cache'] = array(
    '#type' => 'fieldset',
    '#title' => t('Clear cached data'),
    '#description' => t('This module uses cache to store information downloaded from campaign monitor. The information cache is lists datails and custom fields. To re-download the information, clear the cache.'),
  );

  $form['clear_cache']['clear'] = array(
    '#type' => 'submit',
    '#value' => t('Clear cached data'),
    '#submit' => array('campaignmonitor_clear_cache_submit'),
  );

  // Add extra form funtion callbacks.
  $form['#validate'][] = 'campaignmonitor_admin_settings_form_validate';
  $form['#submit'][] = 'campaignmonitor_admin_settings_form_submit';

  return system_settings_form($form);
}

/**
 * Validate the admin settings and load the saved lists.
 *
 * @param array $form
 * @param array $form_state
 */
function campaignmonitor_admin_settings_form_validate($form, &$form_state) {
  // No lists selected or first time here.
  if (!isset($form_state['values'][CM_LISTS])) {
    return;
  }

  $lists = array();
  foreach ($form_state['values'][CM_LISTS] as $form_list) {
    $list = new stdClass();
    $list->id = $form_list['info']['list_id'];
    $list->name = $form_list['info']['name'];
    $list->oncontactpage = $form_list['oncontactpage'];
    $list->onregopage = $form_list['onregopage'];
    $list->onuserpage = $form_list['onuserpage'];
    $list->displayname = $form_list['displayname'];
    $list->namekey = $form_list['namekey'];


    $customfields = array();
    if (isset($form_list[CM_LISTS_CF]) && is_array($form_list[CM_LISTS_CF])) {
      foreach ($form_list[CM_LISTS_CF] as $cf_list) {
        $customfield = new stdClass();
        $customfield->id = $cf_list['cf_id'];
        $customfield->display = $cf_list['display'];
        $customfields[$cf_list['cf_id']] = $customfield;
      }
      $list->customfields = $customfields;
    }

    $lists[$form_list['info']['list_id']] = $list;
  }

  // Remove lists from the form_state.
  unset($form_state['values'][CM_LISTS]);
  variable_set(CM_LISTS, $lists);
}

/**
 * Administration form submit function blocks are rebuild as we've possibly
 * changed blocks.
 */
function campaignmonitor_admin_settings_form_submit($form, &$form_state) {
  _block_rehash();
}

/**
 * Clear the caches used by campaign monitor.
 */
function campaignmonitor_clear_cache_submit($form, &$form_state) {
  cache_clear_all('campaignmonitor_custom_fields', 'cache');
  cache_clear_all('campaignmonitor_list_detail', 'cache');
  cache_clear_all('campaignmonitor_lists', 'cache');
  drupal_set_message(t('Campaign Monitor caches cleared.'));
}