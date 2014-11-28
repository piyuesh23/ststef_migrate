<?php
/**
 * @file
 * Posts Migration Class.
 *
 */

class ststefPostsMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Migrate ststef Blog Posts');

    // Query the legacy db to get the user data
    $query = db_select('st_legacy1.wp_posts','s')
      ->fields('s', array('ID', 'post_author', 'post_date', 'post_date_gmt', 'post_content', 'post_title', 'post_excerpt', 'post_status', 'comment_status', 'ping_status', 'post_password', 'post_name', 'to_ping', 'pinged', 'post_modified', 'post_modified_gmt', 'post_content_filtered', 'post_parent', 'guid', 'menu_order', 'post_type', 'post_mime_type', 'comment_count'))
      ->condition('post_type', 'post')
      ->orderBy('post_date');

    $this->source = new MigrateSourceSQL($query);

    // Machine name of the content-type the data needs to be migrated into.
    $this->destination = new MigrateDestinationNode('blog');

    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'ID' => array(
            'type' => 'int',
            'unsigned' => TRUE,
            'not null' => TRUE,
            'description' => t('Content id from old site'),
            'alias' => 's',
          )
        ),
        MigrateDestinationNode::getKeySchema()
    );

    $this->highwaterField = array(
      'name' => 'post_date', // Column to be used as highwater mark
      'alias' => 's',           // Table alias containing that column
    );

    $this->addFieldMapping('title', 'post_title');
    $this->addFieldMapping('body', 'post_content');
    $this->addFieldMapping('language', 'language_code')->defaultValue('en');
    $this->addFieldMapping('tnid', 'trid');

    $this->addFieldMapping('body:format')
      ->defaultValue('full_html');
    $this->addFieldMapping('created', 'post_date');
    $this->addFieldMapping('uid', 'post_author')
      ->sourceMigration('ststefUsers')
      ->defaultValue(1);
    $this->addFieldMapping('changed', 'post_modified');
    $this->addFieldMapping('status', 'post_status')
      ->defaultValue(1);
    $this->addFieldMapping('promote')
      ->defaultValue(0);
    $this->addFieldMapping('field_tags', 'to_ping')
      ->separator('|')
      ->sourceMigration('ststefTags')
      ->arguments(array('source_type' => 'tid', 'create_term' => TRUE));
    $this->addFieldMapping('field_blog_categories', 'post_password')
      ->separator('|')
      ->sourceMigration('ststefCategories')
      ->arguments(array('source_type' => 'tid', 'create_term' => TRUE));
    $this->addFieldMapping('field_blog_image', 'image_id');
    $this->addFieldMapping('field_blog_image:file_class')
      ->defaultValue('MigrateFileFid');
    $this->addFieldMapping('metatag_title', 'meta_title');
    $this->addFieldMapping('metatag_description', 'metatag_description');
    $this->addFieldMapping('metatag_keywords', 'metatag_keywords');
  }

  public function prepareRow($currentRow) {
    $currentRow->post_date = strtotime($currentRow->post_date) > 0 ? strtotime($currentRow->post_date) : 0;
    $currentRow->post_modified = strtotime($currentRow->post_modified) > 0 ? strtotime($currentRow->post_modified) : 0;
    $currentRow->post_status = $currentRow->post_status == "publish" ? 1 : 0;
    $currentRow->post_content = str_replace('http://blog.st-stefanus.com/', '/', $currentRow->post_content);

    // Fetching tags the current blog is tagged with.
    $post_tags_query = db_select('st_legacy1.wp_term_relationships','st_wtr');
    $post_tags_query->fields('st_wtr', array('term_taxonomy_id'));
    $post_tags_query->condition('object_id', $currentRow->ID);
    $post_tags_query->join('st_legacy1.wp_term_taxonomy', 'st_wtt', 'st_wtr.term_taxonomy_id = st_wtt.term_taxonomy_id');
    $post_tags_query->condition('st_wtt.taxonomy', 'post_tag');
    $post_tags = $post_tags_query->execute()->fetchAllKeyed();
    $post_tags_string = '';

    foreach(array_keys($post_tags) as $post_tag) {
      $post_tags_string .= $post_tag . '|';
    }

    $currentRow->to_ping = rtrim($post_tags_string, '|');

    // Fetching categories the current blog is tagged with.
    $post_categories_query = db_select('st_legacy1.wp_term_relationships','st_wtr');
    $post_categories_query->fields('st_wtr', array('term_taxonomy_id'));
    $post_categories_query->condition('object_id', $currentRow->ID);
    $post_categories_query->join('st_legacy1.wp_term_taxonomy', 'st_wtt', 'st_wtr.term_taxonomy_id = st_wtt.term_taxonomy_id');
    $post_categories_query->condition('st_wtt.taxonomy', 'category');
    $post_categories = $post_categories_query->execute()->fetchAllKeyed();

    $post_category_string = '';

    foreach(array_keys($post_categories) as $post_category) {
      $post_category_string .= $post_category . '|';
    }

    $currentRow->post_password = rtrim($post_category_string, '|');

    // Fetch language & translation id for the current row
    $translation_query = db_select('st_legacy1.wp_icl_translations','st_wit');
    $translation_query->fields('st_wit', array('trid', 'language_code'));
    $translation_query->condition('element_id', $currentRow->ID);
    $translation_query->condition('element_type', 'post_post');
    $translations = $translation_query->execute()->fetchAssoc();

    $currentRow->trid = $translations['trid'];
    $currentRow->language_code = $translations['language_code'];

    // Fix for langcode inconsistency between wordpress & Drupal.
    if ($currentRow->language_code == 'en') {
      $currentRow->language_code = 'en-gb';
    }
    elseif ($currentRow->language_code == 'us') {
      $currentRow->language_code = 'en';
    }

    // Fetch the SEO page-title for the post
    $meta_content_query = db_select('st_legacy1.wp_postmeta', 'wp');
    $meta_content_query->fields('wp', array('meta_value'));
    $meta_content_query->condition('meta_key', '_yoast_wpseo_title');
    $meta_content_query->condition('post_id', $currentRow->ID);
    $currentRow->meta_title = $meta_content_query->execute()->fetchField();

    // Fetch the SEO meta-description for the post
    $meta_content_query = db_select('st_legacy1.wp_postmeta', 'wp');
    $meta_content_query->fields('wp', array('meta_value'));
    $meta_content_query->condition('meta_key', '_yoast_wpseo_metadesc');
    $meta_content_query->condition('post_id', $currentRow->ID);
    $currentRow->metatag_description = $meta_content_query->execute()->fetchField();

    // Fetch the SEO meta-keywords for the post
    $meta_content_query = db_select('st_legacy1.wp_postmeta', 'wp');
    $meta_content_query->fields('wp', array('meta_value'));
    $meta_content_query->condition('meta_key', '_yoast_wpseo_focuskw');
    $meta_content_query->condition('post_id', $currentRow->ID);
    $currentRow->metatag_keywords = $meta_content_query->execute()->fetchField();

    // Fetch images for the blog posts
    $image_url_source_query = db_select('st_legacy1.wp_posts', 'st_wp');
    $image_url_source_query->fields('st_wp', array('guid'));
    $image_url_source_query->condition('post_type', 'attachment');
    $image_url_source_query->condition('post_parent', $currentRow->ID);
    $image_url_source_query->range(0,1);
    $image_url_source = $image_url_source_query->execute()->fetchField();

    if ($image_url_source) {
      $file = $this->fetch_image($image_url_source, 'public://blog_images', TRUE, FILE_EXISTS_RENAME);
      if ($file) {
        $currentRow->image_id = $file->fid;
      }
    }

    return TRUE;
  }

  public function fetch_image($url, $destination, $managed, $replace = FILE_EXISTS_RENAME) {
    $parsed_url = parse_url($url);
    if (!isset($destination)) {
    $path = file_build_uri(drupal_basename($parsed_url['path']));
    }
    else {
      if (is_dir(drupal_realpath($destination))) {
        // Prevent URIs with triple slashes when glueing parts together.
        $path = str_replace('///', '//', "$destination/") . drupal_basename($parsed_url['path']);
      }
      else {
        $path = $destination;
      }
    }
    $local = '';
    $result = drupal_http_request($url);
    if ($result->code == 200) {
      $local = $managed ? file_save_data($result->data, $path, $replace) : file_unmanaged_save_data($result->data, $path, $replace);
    }

    return $local;
  }
}
