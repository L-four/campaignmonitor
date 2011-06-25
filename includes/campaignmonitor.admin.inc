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
function campaignmonitor_admin() {
  $api_key = variable_get(CM_API_KEY, FALSE);
  $client_id = variable_get(CM_CLIENT_ID, '');

  $collapsed = FALSE;
  $collapsible = FALSE;
  if ($api_key) {
    $collapsed = TRUE;
    $collapsible = TRUE;
  }

  $form['campaignmonitor_account_details'] = array(
      '#type' => 'fieldset',
      '#collapsible' => $collapsible,
      '#collapsed' => $collapsed,
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
        '#collapsible' => $collapsible,
        '#collapsed' => $collapsed,
        '#title' => t('General Options'),
    );

    $form['campaignmonitor_general_options'][CM_DISPLAY_ON] = array(
        '#type' => 'checkboxes',
        '#title' => t('Display Options'),
        '#default_value' => variable_get(CM_DISPLAY_ON, array()),
        '#options' => array(
            'contact' => t('Contact Page'),
            'registration' => t('Registration Page')
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

    // Load lists form campaign monitor and default values.
    $available_lists = _campaignmonitor_get_lists($api_key, $client_id);
    $saved_lists = variable_get(CM_LISTS, '');

    $collapsed = FALSE;
    $collapsible = TRUE;

    $form[CM_LISTS] = array(
        '#type' => 'fieldset',
        '#collapsible' => $collapsible,
        '#collapsed' => $collapsed,
        '#title' => t('Available Lists'),
        '#tree' => TRUE,
    );

    foreach ($available_lists as $list_id => $list_name) {
      $saved_list = $saved_lists[$list_id];

      $list_detail = _campaignmonitor_get_list_detail($api_key, $list_id);
      $list_custom_fields = _campaignmonitor_get_custom_fields($api_key, $list_id);

      $details = t('<p>Confirm Opt in: %optin<br/>Unsubscribe Page: %unsub<br/>Confirmation Success Page: %conf</p>', array('%optin' => $list_detail['ConfirmOptIn'], '%unsub' => $list_detail['UnsubscribePage'], '%conf' => $list_detail['ConfirmationSuccessPage']));

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id] = array(
          '#type' => 'fieldset',
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
          '#title' => $list_name,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['list_id'] = array(
          '#type' => 'value',
          '#value' => $list_id,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['name'] = array(
          '#type' => 'value',
          '#value' => $list_name,
      );

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['list_detail'] = array(
          '#value' => $details,
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

      /* $form[CM_LISTS]['campaignmonitor_list_'. $list_id]['displayname'] = array(
        '#type'       => 'checkbox',
        '#title'      => t('Display Name field'),
        '#description' => t('When subscribing, should the name field be displayed.'),
        '#default_value'  => $saved_list->displayname,
        ); */

      $keyoptions = campaignmonitor_get_field_keys();

      $form[CM_LISTS]['campaignmonitor_list_' . $list_id]['namekey'] = array(
          '#type' => 'select',
          '#title' => t('Name'),
          '#options' => $keyoptions,
          '#default_value' => $saved_list->namekey,
      );

      if (count($list_custom_fields) > 0) {

        $form[CM_LISTS]['campaignmonitor_list_' . $list_id][CM_LISTS_CF] = array(
            '#type' => 'fieldset',
            '#collapsible' => TRUE,
            '#collapsed' => TRUE,
            '#title' => t('Custom Fields'),
        );

        $saved_cfs = $saved_list->customfields;

        foreach ($list_custom_fields as $cf) {
          $key = str_replace('[', '', $cf['Key']);
          $key = str_replace(']', '', $key);
          $saved_cf = $saved_cfs[$key];
          if ($saved_cfs == NULL) {
            $default_cfs = TRUE;
          }
          else {
            $default_cfs = $saved_cf->display;
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

  // Add extra form funtion callbacks.
  $form['#validate'][] = 'campaignmonitor_admin_settings_validate';
  $form['#submit'][] = 'campaignmonitor_admin_submit';

  return system_settings_form($form);
}

/**
 * Validate the admin settings and load the saved lists.
 *
 * @param array $form
 * @param array $form_state
 */
function campaignmonitor_admin_settings_validate($form, &$form_state) {
  // no lists selected or first time here
  if (!is_array($form_state['values'][CM_LISTS])) {
    return;
  }

  $lists = array();
  foreach ($form_state['values'][CM_LISTS] as $form_list) {
    $list = new stdClass();
    $list->id = $form_list['list_id'];
    $list->name = $form_list['name'];
    $list->oncontactpage = $form_list['oncontactpage'];
    $list->onregopage = $form_list['onregopage'];
    //$list->displayname = $form_list['displayname'];
    $list->namekey = $form_list['namekey'];


    $customfields = array();
    if (is_array($form_list[CM_LISTS_CF])) {
      foreach ($form_list[CM_LISTS_CF] as $cf_list) {
        $customfield = new stdClass();
        $customfield->id = $cf_list['cf_id'];
        $customfield->display = $cf_list['display'];
        $customfields[$cf_list['cf_id']] = $customfield;
      }
      $list->customfields = $customfields;
    }

    $lists[$form_list['list_id']] = $list;
  }

  // remove lists from the form_state
  unset($form_state['values'][CM_LISTS]);
  variable_set(CM_LISTS, $lists);
}

/**
 * Administration form submit function blocks are rebuild as we've possibly
 * changed blocks.
 *
 * @param array $form
 * @param array $form_state
 */
function campaignmonitor_admin_submit($form, &$form_state) {
  _block_rehash();
}