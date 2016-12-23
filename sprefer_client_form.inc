<?php

function sprefer_client_form($form, &$form_state, $machine_name) {

  if (!empty($form_state['values'])) {
    $values = $form_state['values'];
  }

  // Store machine name.
  if (empty($form_state['machine_name'])) {
    $form_state['machine_name'] = $machine_name;
  }

  $form['#prefix'] = '<div id="sprefer_ajax_form">';
  $form['#suffix'] = '</div>';

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Naam',
    '#attributes' => array(
      'placeholder' => 'Bij voorkeur voornaam en achternaam',
    ),
    '#required' => TRUE,
  );

  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mailadres',
    '#required' => TRUE,
  );

  $form['telephone'] = array(
    '#type' => 'textfield',
    '#title' => 'Telefoonnummer',
    '#attributes' => array(
      'placeholder' => 'Bij voorkeur je 06 nummer',
    ),
    '#required' => TRUE,
  );

  $form['postal_code'] = array(
    '#type' => 'textfield',
    '#title' => 'Postcode',
    '#name' => 'postal_code',
    '#required' => TRUE,
  );

  $form['house_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Huisnummer',
    '#name' => 'house_number',
    '#required' => TRUE,
  );

  if (!empty($values['postal_code']) && !empty($values['house_number'])) {
    $key = variable_get('sprefer_pro6pp_key', '');
    $pro6pp = sprefer_pcapi_lookup($key, $values['postal_code'], $values['house_number']);
    if ($pro6pp['status'] === 'ok') {
      $address = array_pop($pro6pp['results']);
      $form_state['address'] = $address;
      $form['address_check'] = array(
        '#markup' => '<p>Is het adres correct?</p><p>' . check_plain($values['name']) . '<br/>' . $address['street'] . ' ' . check_plain($values['house_number']) . '<br/>' . $address['city'] . '</p>'
      );
    }
    else {
      $form['address_check'] = array(
        '#markup' => '<p>Er kon geen adres worden gevonden bij deze postcode en huisnummer combinatie!</p>'
      );
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Verder',
    '#name' => 'submit',
    '#ajax' => array(
      'wrapper' => 'sprefer_ajax_form',
      'callback' => 'sprefer_form_ajax_callback',
    ),
  );

  return $form;
}

function sprefer_client_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  preg_match('/^([1-9][0-9]{3})[\s]?([A-Za-z]{2})$/', $values['postal_code'], $matches); 
  $postal_code = $matches[1] . ' ' . strtoupper($matches[2]);
  $values['postal_code'] = $postal_code;

  // Check email.
  if (valid_email_address($values['email'])) {
    $email_errors = _sp_email_verify_check($values['email']);
    if (!empty($email_errors)) {
      $mail_error = TRUE;
    }
  }
  else {
    $mail_error = TRUE;
  }
  if (!empty($mail_error)) {
    form_set_error('email', 'Vul een geldig e-mail adres in. Als het adres geldig is, maar desondanks niet geaccepteerd wordt, neem dan contact op met webmaster@sp.nl.');
  }
  // Check postal code.
  if (!preg_match('/^[1-9][0-9]{3}[\s]?[A-Za-z]{2}$/', $values['postal_code'])) {
    form_set_error('postal_code', 'Vul een geldige postcode in!');
  }

  // Check phone.
  $regex_telephone_fixed = '/^(((0)[1-9]{2}[0-9][-]?[1-9][0-9]{5})|((\\+31|0|0031)[1-9][0-9][-]?[1-9][0-9]{6}))$/';
  $regex_telephone_mobile = '/^(((\\+31|0|0031)6){1}[1-9]{1}[0-9]{7})$/';
  if (!(preg_match($regex_telephone_fixed, $values['telephone']) || preg_match($regex_telephone_mobile, $values['telephone']))) {
    form_set_error('telephone', 'Vul een geldig telefoonnummer in!');
  }
}

function sprefer_client_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];
  // Check if house number and postal code are verified.
  if (!empty($storage['postal_code']) && !empty($storage['house_number']) && $values['postal_code'] == $storage['postal_code'] && $values['house_number'] == $storage['house_number']) {
    $key = variable_get('sprefer_pro6pp_key', '');
    $pro6pp = sprefer_pcapi_lookup($key, $values['postal_code'], $values['house_number']);
    if ($pro6pp['status'] === 'ok') {
      // Adres is ok.
      // Get CiviCRM contact id.
      $site_name = variable_get('site_name');
      $sprefer = sprefer_get_sprefer($form_state['machine_name']);
      $source = 'SP refer: ' . $site_name . '(Blok: ' . $sprefer->display_name . ')';
      $result = spcivipush_push_contact($values, array(), 'sprefer', $source);
      if ($result === FALSE) {
        drupal_set_message('Er kon geen contactinformatie worden opgehaald. Probeer het op een later moment nog eens.', 'error');
      }
      elseif ($result === 'insufficient data') {
        drupal_set_message('Er is niet genoeg informatie om een contact aan te maken.', 'error');
      }
      else {
        // CiviCRM contact gevonden.
        $contact_id = $result;

        // Send e-mail.
        $hash = urlencode(sprefer_hash($contact_id));
        $site_email = variable_get('site_mail', 'webmaster@sp.nl');

        // Replace tokens in mail subject and text.
        $params = array(
          'mail_subject' => $sprefer->mail_subject,
          'mail_text' => $sprefer->mail_text,
        );

        $tokens = array(
          'name' => check_plain($values['name']),
          'email' => check_plain($values['email']),
          'telephone' => check_plain($values['telephone']),
          'postal_code' => check_plain($values['postal_code']),
          'house_number' => check_plain($values['house_number']),
          'street' => $form_state['address']['street'],
          'city' => $form_state['address']['city'],
          'hash' => $hash,
          'contact_id' => $contact_id,
        );
        foreach ($tokens as $token_key => $token_value) {
          $params = str_replace('[' . $token_key . ']', check_plain($token_value), $params);
        }

        $message = drupal_mail('sprefer', 'client_mail', $values['email'], language_default(), $params, $site_email); 
        // Redirect to succes page.
        ctools_include('ajax');
        ctools_add_js('ajax-responder');
        $data = array(
          'name' => $values['name'],
        );
        $hash = sprefer_hash($data, TRUE);
        $commands[] = ctools_ajax_command_redirect('sprefer/success', 0, array(
          'query' => array(
            'data' => $hash,
          ),
        ));
        print ajax_render($commands);
        exit;
      }
    }
  }
  $storage['house_number'] = $values['house_number'];
  $storage['postal_code'] = $values['postal_code'];
  $form_state['rebuild'] = TRUE;
}
