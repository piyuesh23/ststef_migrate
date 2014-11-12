<?php

/**
 * Implements hook_migrate_api()
 */
function ststef_migrate_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'ststef' => array(
        'title' => t('ststef Migrations'),
      ),
    ),
    'migrations' => array(
      'ststefPosts' => array(
        'class_name' => 'ststefPostsMigration',
        'group_name' => 'ststef',
      ),
      'ststefComments' => array(
        'class_name' => 'ststefCommentsMigration',
        'group_name' => 'ststef',
      ),
      'ststefUsers' => array(
        'class_name' => 'ststefUsersMigration',
        'group_name' => 'ststef',
      ),
      'ststefTags' => array(
        'class_name' => 'ststefTagsMigration',
        'group_name' => 'ststef',
      ),
      'ststefCategories' => array(
        'class_name' => 'ststefCategoriesMigration',
        'group_name' => 'ststef',
      ),
    ),
  );
  return $api;
}
