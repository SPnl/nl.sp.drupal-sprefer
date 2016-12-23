<?php

function sprefer_admin_form($form, &$form_state) {
  $form['#prefix'] = '<div id="sprefer_ajax_form">';
  $form['#suffix'] = '</div>';

  $form['sprefer_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'SP Refer instellingen',
  );

  $default = variable_get('sprefer_pro6pp_key', '');
  $form['sprefer_settings']['pro6pp_key'] = array(
    '#type' => 'textfield',
    '#title' => 'Pro6PP sleutel',
    '#default_value' => $default,
  );

  $form['sprefer_settings']['settings_submit'] = array(
    '#type' => 'submit',
    '#value' => 'Sla op',
    '#name' => 'settings_submit',
    '#ajax' => array(
      'wrapper' => 'sprefer_ajax_form',
      'callback' => 'sprefer_form_ajax_callback',
    ),
  );

  $form['sprefer_blocks'] = array(
    '#type' => 'fieldset',
    '#title' => 'SP Refer blokken',
  );

  $options = sprefer_get_refer_forms_list();
  $form['sprefer_blocks']['select_form'] = array(
    '#type' => 'select',
    '#title' => 'Bewerk een bestaand sprefer blok of maak een nieuw blok aan.',
    '#name' => 'select_form',
    '#options' => $options,
    '#empty_option' => 'Nieuw:',
    '#ajax' => array(
      'wrapper' => 'sprefer_ajax_form',
      'callback' => 'sprefer_form_ajax_callback',
    ),
  );

  $form['sprefer_blocks']['display_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Blok titel (voor beheerscherm)',
  );

  $access = empty($form_state['values']['select_form']) ? TRUE : FALSE;
  $form['sprefer_blocks']['machine_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Blok machine naam',
    '#access' => $access,
  );

  $form['sprefer_blocks']['mail_subject'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail onderwerp',
    '#maxlength' => 255,
    '#description' => '<p>De volgende tokens zijn bruikbaar:<br/>[name], [email], [telephone], [postal_code], [house_number], [street], [city], [hash], [contact_id].',
  );

  $form['sprefer_blocks']['mail_text'] = array(
    '#type' => 'textarea',
    '#title' => 'E-mail tekst',
    '#description' => '<p>De volgende tokens zijn bruikbaar:<br/>[name], [email], [telephone], [postal_code], [house_number], [street], [city], [hash], [contact_id].',
  );

  $form['sprefer_blocks']['feedback_page_title'] = array(
    '#type' => 'textfield',
    '#title' => 'Feedback pagina titel',
    '#maxlength' => 255,
    '#description' => '<p>De volgende tokens zijn bruikbaar:<br/>[name].',
  );

  $form['sprefer_blocks']['feedback_page_text'] = array(
    '#type' => 'textarea',
    '#title' => 'Feedback pagina tekst',
    '#description' => '<p>De volgende tokens zijn bruikbaar:<br/>[name].',
  );

  $form['sprefer_blocks']['blocks_submit'] = array(
    '#type' => 'submit',
    '#name' => 'blocks_submit',
  );

  if (!empty($form_state['values']['select_form'])) {
    $form['sprefer_blocks']['blocks_delete'] = array(
      '#type' => 'submit',
      '#value' => 'Verwijder',
      '#name' => 'blocks_delete',
    );

    $sprefer = sprefer_get_sprefer($form_state['values']['select_form']);
    $form_state['input']['display_name'] = $sprefer->display_name;
    $form_state['input']['machine_name'] = $sprefer->machine_name;
    $form_state['input']['mail_text'] = $sprefer->mail_text;
    $form_state['input']['mail_subject'] = $sprefer->mail_subject;
    $form_state['input']['feedback_page_text'] = $sprefer->feedback_page_text;
    $form_state['input']['feedback_page_title'] = $sprefer->feedback_page_title;
    $form['sprefer_blocks']['blocks_submit']['#value'] = 'Sla op';
  }
  else {
    $form_state['input']['display_name'] = '';
    $form_state['input']['machine_name'] = '';
    $form_state['input']['mail_text'] = '';
    $form_state['input']['mail_subject'] = '';
    $form_state['input']['feedback_page_text'] = '';
    $form_state['input']['feedback_page_title'] = '';
    $form['sprefer_blocks']['blocks_submit']['#value'] = 'Voeg toe';
  }
  
  return $form;
}

function sprefer_admin_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'blocks_submit':
      if (empty($values['select_form']) && !preg_match('/^[a-z0-9_]+$/', $values['machine_name'])) {
        form_set_error('machine_name', 'Vul een geldige machine naam in (kleine letters, cijfers en underscores).');
      }
      if (empty($values['select_form']) && !empty(sprefer_get_sprefer($values['machine_name']))) {
        form_set_error('machine_name', 'De gekozen machine naam is reeds in gebruik.');
      }
    }
  }
}

function sprefer_admin_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'settings_submit':
      // Store Pro6PP key.
      variable_set('sprefer_pro6pp_key', $values['pro6pp_key']);
      $form_state['rebuild'] = TRUE;
      drupal_set_message('Pro6PP sleutel is opgeslagen');
      break;
    case 'blocks_submit':
      // Store blocks.
      $machine_name = !empty($values['select_form']) ? $values['select_form'] : $values['machine_name'];
      if (empty($values['select_form'])) {
        $refer_form = entity_create('sprefer_form', array(
          'display_name' => $values['display_name'],
          'machine_name' => $values['machine_name'],
          'disabled' => 0,
          'mail_subject' => $values['mail_subject'],
          'mail_text' => $values['mail_text'],
          'feedback_page_title' => $values['feedback_page_title'],
          'feedback_page_text' => $values['feedback_page_text'],
        ));
        $result = entity_save('sprefer_form', $refer_form);
      }
      else {
        $sprefer = sprefer_get_sprefer($machine_name);
        $sprefer->display_name = $values['display_name'];
        $sprefer->mail_subject = $values['mail_subject'];
        $sprefer->mail_text = $values['mail_text'];
        $sprefer->feedback_page_title = $values['feedback_page_title'];
        $sprefer->feedback_page_text = $values['feedback_page_text'];
        $result = entity_save('sprefer_form', $sprefer);
      }
      if ($result === FALSE) {
        drupal_set_message('Er is een fout opgetreden bij het opslaan van de gegevens.', 'error');
        $form_state['rebuild'] = TRUE;
        return;
      }
      else {
        drupal_set_message('SP Refer blok is opgeslagen.');
      }
      break;
    case 'blocks_delete':
      $sprefer = sprefer_get_sprefer($values['select_form']);
      $result = entity_delete('sprefer_form', $sprefer->id);
      if ($result === FALSE) {
        drupal_set_message('Er is een fout opgetreden bij het verwijderen van de gegevens.', 'error');
      }
      else {
        drupal_set_message('SP Refer blok is verwijderd.');
      }
      break;
    }
  }
}
