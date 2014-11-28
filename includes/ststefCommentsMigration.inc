<?php

/**
 * @file
 * Comments Migration Class.
 *
 */
class ststefCommentsMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = 'Comments about Download';
    $this->map = new MigrateSQLMap($this->machineName,
      array('comment_ID' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      )
      ),
      MigrateDestinationComment::getKeySchema()
    );
    $query = db_select('st_legacy1.wp_comments', 'c')
      ->fields('c', array('comment_ID', 'comment_post_ID', 'comment_author', 'comment_author_email', 'comment_author_url', 'comment_author_IP', 'comment_date', 'comment_date_gmt', 'comment_content', 'comment_karma', 'comment_approved', 'comment_agent', 'comment_type', 'comment_parent', 'user_id'));
    $query->join('st_legacy1.wp_posts','st_wp','st_wp.ID = c.comment_post_ID');
    $query->condition('st_wp.post_type', 'post');
    $query->condition('c.comment_approved', '1');
    $query->orderBy('comment_post_ID');

    $this->source = new MigrateSourceSQL($query);
    $this->destination = new MigrateDestinationComment('comment_node_blog');
    $generic_textarea_arguments = MigrateTextFieldHandler::arguments(NULL, filter_default_format());

    $this->addFieldMapping('hostname', 'comment_author_IP');
    $this->addFieldMapping('subject')->defaultValue('');
    $this->addFieldMapping('mail', 'comment_author_email');
    $this->addFieldMapping('status', 'comment_approved');
    $this->addFieldMapping('nid', 'comment_post_ID')
      ->sourceMigration('ststefPosts');
    $this->addFieldMapping('uid', 'user_id')
      ->sourceMigration('ststefUsers')
      ->defaultValue(0);
    $this->addFieldMapping('pid', 'comment_parent')
      ->sourceMigration('ststefComments');
    $this->addFieldMapping('comment_body', 'comment_content')
      ->arguments($generic_textarea_arguments);
    $this->addFieldMapping('created', 'comment_date');
    $this->addFieldMapping('changed', 'comment_date');
    $this->addFieldMapping('homepage', 'comment_author_url');
    $this->addFieldMapping('name', 'comment_author');

    // No unmapped source fields

    // Unmapped destination fields
    $this->addFieldMapping('thread')
      ->issueGroup(t('DNM'));
    $this->addFieldMapping('language')
      ->issueGroup(t('DNM'));
  }

  public function prepareRow($currentRow) {
    $currentRow->comment_date = strtotime($currentRow->comment_date) > 0 ? strtotime($currentRow->comment_date) : time();
    $currentRow->comment_author = substr($currentRow->comment_author, 0, 60);
    $currentRow->comment_approved = $currentRow->comment_approved == 1 ? 1 : 0;
  }
}
