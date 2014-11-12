<?php
/**
 * @file
 * Posts Tags Migration Class.
 *
 */

class ststefTagsMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Migrate ststef Tags');

    // Query the legacy db to get the user data
    $query = db_select('st_legacy.wp_terms', 't');
    $query->fields('t', array('name', 'term_id'));
    $query->join('st_legacy.wp_term_taxonomy', 'tt', 'tt.term_id = t.term_id');
    $query->condition('tt.taxonomy', 'post_tag');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'term_id' => array(
          'type' => 'int',
          'length' => 20,
          'not null' => TRUE,
          'description' => 'Term Id from legacy table',
          'alias' => 't',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceSQL($query);

    // Set up our destination
    $this->destination = new MigrateDestinationTerm('tags');

    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('description')
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('format')
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('weight')
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('parent')
      ->issueGroup(t('DNM'));
  }

  public function prepareRow($currentRow) {
    return TRUE;
  }
}
