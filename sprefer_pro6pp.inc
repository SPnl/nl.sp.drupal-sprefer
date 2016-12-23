<?php

/**
 * Retrieve street name and city belonging to postal code and house number.
 *
 * This function makes a call to the pro6pp.nl API to get the street name and
 * city that is associated with the postal code and house number.
 *
 * @param string $key
 *   The key that should be used to identify at the pro6pp.nl API service.
 * @param string $postal_code
 *   The postal code of the address.
 * @param string $house_number
 *   The house number of the address.
 *
 * @return string[]
 *   The name of the street can be found at the index 'street' and the name of
 *   the name of the city at the index 'city' in the array.
 */
function sprefer_pcapi_lookup($key, $postal_code, $house_number) {
  // First, parse the postal code and house number (which should split the
  // addition).
  $postal_code = _sprefer_parse_postal_code($postal_code);
  list($house_number, $house_number_addition) = _sprefer_parse_house_number($house_number);

  // Both the postal code and the house number should have a value.
  if (empty($postal_code) || empty($house_number)) {
    return array();
  }

  // We prevent making unnecessary lookups by caching results, 
  // also wrong lookups!
  $cache_key = 'sprefer:' . $postal_code . ':' . $house_number . ':' . $house_number_addition;

  // Use the local memory as the first level cache.
  $data = &drupal_static(__FUNCTION__);
  if (!isset($data)) {
    // When not stored in local memory, check the cache stored in the database.
    if(FALSE) {
    //if ($cache = cache_get($cache_key)) {
      $data = $cache->data;
    }
    else {
      // Construct the URL to perform the lookup with.
      $url = 'https://api.pro6pp.nl/v1/autocomplete?auth_key='.$key.'&nl_sixpp='.$postal_code.'&format=json';
      $response = drupal_http_request(url($url));

      // When the lookup went correctly, parse the JSON data.
      if ($response->code == 200) {
        $data = drupal_json_decode($response->data);
      }

      // Store the result in the cache of the database.
      //
      // NB: if the lookup didn't responded in a correct result, we still store
      // a 'mark' in the database. In this way also invalid lookups will be
      // served from cache the next time.
      cache_set($cache_key, $data, 'cache', time() + 900);
    }
  }

  return $data;
}

/**
 * Parse a postal code to be used in the request to pro6pp.nl API.
 *
 * @param string $postal_code
 *   The postal code that should be normalized.
 *
 * @return string
 *   The normalized postal code.
 */
function _sprefer_parse_postal_code($postal_code) {
  // First, remove any whitespaces.
  $postal_code = str_replace(' ', '', trim($postal_code));

  // If the postal code is in the form of 1234AB, than return the trimmed
  // version.
  if ((boolean) preg_match('~^\d{4}[[:alpha:]]{2}$~', $postal_code)) {
    return $postal_code;
  }

  return '';
}

/**
 * Parse the house number by separating the number from the additional value.
 *
 * @param string $house_number_full
 *   The house number of the address (including an possible addition).
 *
 * @return string[]
 *   Normalized version of the house number, where the first index of the array
 *   contains the number and the second value is the addition.
 */
function _sprefer_parse_house_number($house_number_full) {
  // Remove white spaces before and after the provided house number (including
  // addition).
  $house_number_full = trim($house_number_full);

  $house_number = '';
  $house_number_addition = '';
  if (preg_match('~^(?P<house_number>\d+)[^[:alnum:]]*(?P<addition>[[:alnum:]]*)$~', $house_number_full, $match)) {
    $house_number = $match['house_number'];
    $house_number_addition = !empty($match['addition']) ? $match['addition'] : '';
  }

  return array($house_number, $house_number_addition);
}
