<?php

/*
 * @file
 * Implementation of the newsletter form on the user page.
 */

/**
 * Builds a newsletter selection form for the user page.
 *
 * @return array $form
 */
function campaignmonitor_user_page_form() {
  global $user;
  // Replace api_key and list_id with your own details
  $api_key = variable_get(CM_API_KEY, '');
  $client_id = variable_get(CM_CLIENT_ID, '');
  $lists = campaignmonitor_get_available_lists();

  $options = array();
  $default_values = array();

  foreach ($lists as $list_id => $list) {
    if ($list != '0') {
      // If the current user is subscribed to the list check the checkbox.
      $default = FALSE;
      if (_campaignmonitor_is_subscribed($api_key, $list_id, $user->mail, TRUE)) {
        $default = TRUE;
        $default_values[] = $list_id;
      }
      $options[$list_id] = $list->name;

      $form['is_subscribed_' . $list_id] = array(
          '#type' => 'hidden',
          '#default_value' => $default,
      );
    }
  }

  $form['subscribe_newsletter'] = array(
      '#type' => 'checkboxes',
      '#title' => variable_get(CM_USERPAGE_DISPLAY_TEXT, CM_USERPAGE_DISPLAY_TEXT_DEFAULT),
      '#options' => $options,
      '#default_value' => $default_values,
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}

/**
 * Implementation of user page form submit.
 */
function campaignmonitor_user_page_form_submit($form, &$form_state) {
  global $user;
  
  // Try to get name from default key values
  $account = campaignmonitor_get_field_key_values($user->uid);
  $name = $account[$list->namekey];

  $api_key = variable_get(CM_API_KEY, '');
  $client_id = variable_get(CM_CLIENT_ID, '');

  // Get the different lists avaliable and their state for the current user.
  $lists = _campaignmonitor_get_lists($api_key, $client_id);
  $chosen_lists = campaignmonitor_get_available_lists();
  $subscribe = $form_state['values']['subscribe_newsletter'];
  
  foreach ($lists as $list_id => $list) {
    if ($list) {
      // if subscribed, add if not already subscribed
      if ($subscribe[$list_id] != NULL && !$form_state['values']['is_subscribed_' . $list_id]) {
        _campaignmonitor_add_subscriber($api_key, $list_id, $name, $user->mail);
      }
      // if not subscribed, and was subscribed before
      elseif ($subscribe[$list_id] == NULL && $form_state['values']['is_subscribed_' . $list_id]) {
        _campaignmonitor_remove_subscriber($api_key, $list_id, $user->mail);
      }
    }
  }
}