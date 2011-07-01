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

  // Only display form if selected in the administration page.
  $display_on = variable_get(CM_DISPLAY_ON, array());

  if ($display_on['userpage']) {
    // Replace api_key and list_id with your own details
    $api_key = variable_get(CM_API_KEY, '');
    $client_id = variable_get(CM_CLIENT_ID, '');
    $lists = campaignmonitor_get_available_lists();

    $options = array();
    $default_values = array();

    foreach ($lists as $list_id => $list) {
      if ($list && $list->onuserpage) {
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

    if (!empty ($options)) {
      $form['subscribe_newsletter'] = array(
          '#type' => 'checkboxes',
          '#title' => t(variable_get(CM_USERPAGE_DISPLAY_TEXT, CM_USERPAGE_DISPLAY_TEXT_DEFAULT)),
          '#options' => $options,
          '#default_value' => $default_values,
      );

      $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
    }
    else {
      $form['subscribe_newsletter'] = array(
        '#value' => t('There are no list avaliable.'),
      );
    }
  }
  else {
    $form['subscribe_newsletter'] = array(
      '#value' => t('You are not allow to subscrib newsletters.'),
    );
  }

  return $form;
}

/**
 * Implementation of user page form submit.
 */
function campaignmonitor_user_page_form_submit($form, &$form_state) {
  global $user;
  
  $api_key = variable_get(CM_API_KEY, '');
  $client_id = variable_get(CM_CLIENT_ID, '');

  // Get the different lists avaliable and their state for the current user.
  $chosen_lists = campaignmonitor_get_available_lists();
  $subscribe = $form_state['values']['subscribe_newsletter'];
  
  foreach ($chosen_lists as $list_id => $list) {
    if ($subscribe[$list_id]) {
      // Try to get name from default key values.
      $account = campaignmonitor_get_field_key_values($user->uid);
      $name = isset($account[$list->namekey]) ? $account[$list->namekey] : '';

      // If subscribed, add if not already subscribed.
      if (isset($subscribe[$list_id]) && !$form_state['values']['is_subscribed_' . $list_id]) {
        _campaignmonitor_add_subscriber($api_key, $list_id, $name, $user->mail);
      }
    }
    // If not subscribed, and was subscribed before.
    elseif (!$subscribe[$list_id] && $form_state['values']['is_subscribed_' . $list_id]) {
      _campaignmonitor_remove_subscriber($api_key, $list_id, $user->mail);
    }
  }
}