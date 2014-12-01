# Pre-requisites:
  - Import Wordpress database with dbname as st_legacy1.
  - Copy over wp_content/uploads in the webroot of SF instance such that the
     content inside this folder is accessible at http://ststef.sabmiller.acsitefactory.com/wp_content/uploads.
  - Enable translation for blog content type since the blogs on live site are multilingual.
  - Create a new vocabulary with machine name 'blog_category'.
  - Add a multivalued field field_blog_category(term reference field) to the blog_entry content type.
  - Add a single valued field field_blog_image to  blog content type.
  - Create a folder 'blog_images' inside sites/default/files & make sure its writeable by user running the server.
# Running Migrations using drush:
  - Before running any of the migrations, make sure we register the migration handlers in the UI. Could have been done via code as well, but since we need to run it from local, manual registration is fine.
    - goto admin/content/import/registration and click on Register statically-defined classes.
    - Clear caches.
  - drush mi ststefUsers -- Few seconds.
  - drush mi ststefTags --feedback="100 items" -- 1 min.
  - drush mi ststefCategories --feedback="10 items" -- 2-3 minutes.
  - drush mi ststefPosts --feedback="50 items" -- Around 10 mins.
  - drush mi ststefComments --feedback="50 items" -- Around 9 secs. Migrates only the approved comments
