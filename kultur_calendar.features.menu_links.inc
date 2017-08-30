<?php
/**
 * @file
 * kultur_calendar.features.menu_links.inc
 */

/**
 * Implements hook_menu_default_menu_links().
 */
function kultur_calendar_menu_default_menu_links() {
  $menu_links = [];

  // Exported menu link: main-menu_calendar:kalender
  $menu_links['main-menu_calendar:kalender'] = [
    'menu_name' => 'main-menu',
    'link_path' => 'kalender',
    'router_path' => 'kalender',
    'link_title' => 'Kalender',
    'options' => [
      'domain_menu_access' => [
        'show' => [
          'd2' => 'd2',
        ],
      ],
      'alter' => TRUE,
      'attributes' => [
        'title' => '',
      ],
      'identifier' => 'main-menu_calendar:kalender',
    ],
    'module' => 'menu',
    'hidden' => 0,
    'external' => 0,
    'has_children' => 0,
    'expanded' => 0,
    'weight' => 0,
    'customized' => 1,
  ];
  // Translatables
  // Included for use with string extractors like potx.
  t('Kalender');


  return $menu_links;
}
