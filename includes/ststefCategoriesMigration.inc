<?php
/**
 * @file
 * Post Categories Migration Class.
 *
 */

class ststefCategoriesMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Migrate ststef categories');

    // Query the legacy db to get the user data
    $query = db_select('st_legacy1.wp_terms', 't');
    $query->fields('t', array('name', 'term_id'));
    $query->fields('tt', array('term_taxonomy_id'));
    $query->join('st_legacy1.wp_term_taxonomy', 'tt', 'tt.term_id = t.term_id');
    $query->condition('tt.taxonomy', 'category');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'term_taxonomy_id' => array(
          'type' => 'int',
          'length' => 20,
          'not null' => TRUE,
          'description' => 'Term Id from legacy table',
          'alias' => 'tt',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceSQL($query);

    // Set up our destination
    $this->destination = new MigrateDestinationTerm('blog_category', array('allow_duplicate_terms' => TRUE));


    $this->addFieldMapping('name', 'name');
    $this->addFieldMapping('language', 'language_code');
    // $this->addFieldMapping('i18n_tsid', 'trid');
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
    // Fetch language & translation id for the current row
    $translation_query = db_select('st_legacy1.wp_icl_translations','st_wit');
    $translation_query->fields('st_wit', array('trid', 'language_code'));
    $translation_query->condition('element_id', $currentRow->term_taxonomy_id);
    $translation_query->condition('element_type', 'tax_category');
    $translations = $translation_query->execute()->fetchAssoc();

    // Decode special HTML entities.
    $currentRow->name = htmlspecialchars_decode($currentRow->name);

    // Remove all @{language_code} from terms.
    $pattern_language_code = '/@[(a-z)]*$/';
    preg_match($pattern_language_code, $currentRow->name, $matches);
    $language_code_unsanitised = array_shift($matches);
    $language_code = str_replace('@', '', $language_code_unsanitised);
    $currentRow->name = str_replace($language_code_unsanitised, '', $currentRow->name);

    $currentRow->trid = $translations['trid'];
    $currentRow->language_code = $translations['language_code'];

    // Fix for langcode inconsistency between wordpress & Drupal.
    if ($currentRow->language_code == 'en') {
      $currentRow->language_code = 'en-gb';
    }
    elseif ($currentRow->language_code == 'us') {
      $currentRow->language_code = 'en';
    }
    return TRUE;
  }
}
