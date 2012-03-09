<?php

/**
 * @file
 * campaignmonitor_commerce
 *
 * This file contains...
 */

/**
 * Payment pane: form callback.
 */
function campaignmonitor_commerce_pane_checkout_form($form, &$form_state, $checkout_pane, $order) {
  $pane_form = array();
  $options   = array();
  $defaults  = array();

  // Get connected to the API and get lists.
  $cm = CampaignMonitor::getConnector();
  $lists = $cm->getLists();

  // Find form key to index the form array and load options.
  $form_key = 'campaignmonitor_commerce_checkout_lists';
  $options = variable_get($form_key, array());
  $campaignmonitor_commerce_enabled = variable_get('campaignmonitor_commerce_enabled', FALSE);

  if (!empty($options)) {

    $count = count($options);

    foreach ($options as $key => $value) {
      $defaults[] = $key;
      $options[$key] = $lists[$key]['name'];
    }

    $pane_form['campaignmonitor_lists'] = array(
      '#type' => 'checkboxes',
      '#description' => format_plural($count, 'Subscribe to the newsletter.', 'Select the news lists, that you want to subscribe to.'),
      '#options' => $options,
      '#default_value' => $defaults,
    );
  }

  return $pane_form;
}


function campaignmonitor_commerce_pane_checkout_form_submit($form, &$form_state, $checkout_pane, $order) {
  if (isset($form_state['values']['campaignmonitor_commerce']['campaignmonitor_lists']) && !form_get_errors()) {
    // Get connected to the API and get lists.
    $cm = CampaignMonitor::getConnector();
    $lists_info = $cm->getLists();
    $name = $form_state['values']['customer_profile_shipping']['commerce_customer_address']['und'][0]['first_name']
          . ' '
          . $form_state['values']['customer_profile_shipping']['commerce_customer_address']['und'][0]['last_name'];
    // Find the selected lists, if any.
    foreach ($form_state['values']['campaignmonitor_commerce']['campaignmonitor_lists'] as $list_id => $selected) {
      if ($selected) {
        // Try to subscribe to the list.
        if (!$cm->subscribe($list_id, check_plain($order->mail), check_plain($name))) {
          $form_state['redirect'] = FALSE;
          return FALSE;
        }
      }
    }
  }
}

/**
 * Checkout pane callback: returns the cart contents pane's settings form.
 */

function campaignmonitor_commerce_pane_settings_form($checkout_pane) {
  $form = array();
  $options = array();

  // Get connected to the API and get lists.
  $cm = CampaignMonitor::getConnector();
  $lists = $cm->getLists();


  foreach ($lists as $uuid => $list) {
    $options[$uuid] = $list['name'];
  }

  // load defaults.
  $form_key = 'campaignmonitor_commerce_checkout_lists';
  $defaults = variable_get($form_key, array());

  // Add options to select witch lists can be subscribed to on the commerce checkout page.
  $form[$form_key] = array(
    '#type' => 'checkboxes',
    '#title' => t('Display list on commerce checkout page'),
    '#options' => $options,
    '#description' => t('Enable this list on the commerce checkout page and allow subscription.'),
    '#default_value' => $defaults,
  );

  // load defaults.
  $form_key = 'campaignmonitor_commerce_enabled';
  $defaults = variable_get($form_key, FALSE);

  $form[$form_key.'_title'] = array(
    '#type' => 'item',
    '#title' => t('Options'),
  );
  // Add option to enable Precheck lists on commerce checkout page.
  $form[$form_key] = array(
    '#type' => 'checkbox',
    '#title' => t('Precheck boxes?'),
    '#description' => t('Enable this to precheck newsletter subscriptions.'),
    '#default_value' => $defaults,
  );

  return $form;
}

/**
 * Implements the callback for the checkout pane review form;
 * Stops errors on saving the plane settings
 * @Todo Find out why this is
 */
function campaignmonitor_commerce_pane_review($form, $form_state, $checkout_pane, $order) {
  return FALSE;
}
