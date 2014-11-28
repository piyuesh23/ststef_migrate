<?php
/**
 * @file
 * Post Users Migration Class.
 *
 */

class ststefUsersMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Migrate St. Stefanus Users');

    // Query the legacy db to get the user data
    $query = db_select('st_legacy1.wp_users','u')
      ->fields('u', array('ID', 'user_login', 'user_pass', 'user_nicename', 'user_email', 'user_url', 'user_registered', 'user_activation_key', 'user_status', 'display_name'))
      ->condition('ID', 1, '<>');

    // Instantiate source
    $this->source = new MigrateSourceSQL($query);

    // Destiation as the migration destination User class
    $this->destination = new MigrateDestinationUser();

    // Mapping between legacy unique id and Drupal user Id.
    $this->map  = new MigrateSQLMap($this->machineName,
      array(
        'ID' => array(
          'type'  => 'int',
          'length' => 20,
          'description' => t('User id from old site'),
          'alias' => 'u',
        )
      ),
      MigrateDestinationUser::getKeySchema()
    );

    // Mappings
    $this->addFieldMapping('name', 'user_login')->dedupe('users', 'name');
    $this->addFieldMapping('pass', 'Password');
    $this->addFieldMapping('mail', 'user_email')->dedupe('users', 'mail');
    $this->addFieldMapping('language')->defaultValue('und');
    $this->addFieldMapping('theme')->defaultValue('');
    $this->addFieldMapping('signature')->defaultValue('');
    $this->addFieldMapping('signature_format')->defaultValue('filtered_html');
    $this->addFieldMapping('status', 'user_status')->defaultValue(1);
    $this->addFieldMapping('picture')->defaultValue(0);
    $this->addFieldMapping('roles')
      ->defaultValue(drupal_map_assoc(array(2)));;
    $this->addFieldMapping('created','user_registered');
    $this->addFieldMapping('access','user_registered');
    $this->addFieldMapping('login','user_registered');
    $this->addFieldMapping('field_profile_display_name', 'display_name');
    $this->addFieldMapping('field_profile_user_nicename', 'user_nicename');
  }

  public function prepareRow($currentRow) {
    return TRUE;
  }
}
