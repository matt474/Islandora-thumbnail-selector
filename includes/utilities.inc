<?php

/**
 * @file
 * Miscellaneous helper functions.
 */
  
  /**
 * A Drupal form select table populated with the given collection's children and displays thier thumbnails.
 *
 * @param AbstractObject $object
 *   The collection object.
 * @param array $pager_options
 *   An associative array containing options to pass onto the pager.
 *   - element: The numeric ID of the pager
 *   - fragment: The url fragment to append onto pager links, includes '#'.
 *
 * @return array
 *   A Drupal select table form element.
 */
function islandora_thumbnail_selector_get_thumbnail_select_form_element(AbstractObject $object, array $pager_options) {
  // Assumes all results are returned although the function description
  // states otherwise.
  module_load_include('inc', 'islandora', 'includes/utilities');
  $limit = variable_get('islandora_basic_collection_admin_page_size', 10);
  $page = pager_find_page($pager_options['element']);
  list($count, $results) = islandora_basic_collection_get_member_objects($object, $page, $limit, 'manage');
  $page = pager_default_initialize($count, $limit, $pager_options['element']);
  $rows = array();
  foreach ($results as $result) {
    $pid = $result['object']['value'];
    $label = empty($result['title']['value']) ? t('Untitled') : filter_xss($result['title']['value']);
    $rows[$pid] = array(
      'thumbnail' => array(
        'data' => array(
          '#markup' => '<div>' . theme('image', array('path' => 'islandora/object/' . $pid . '/datastream/TN')) . '</div>',
        ),
      ),
      'label' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $label,
          '#href' => "islandora/object/{$pid}")));
  }
  $pager = theme('pager', array('quantity' => $limit, 'element' => $pager_options['element']));
  $pager = islandora_basic_collection_append_fragment_to_pager_url($pager, $pager_options['fragment']);
  return array(
    '#type' => 'tableselect',
    '#header' => array(
      'thumbnail' => array('data' => t('Thumbnail')),
      'label' => array('data' => t('Label')),
    ),
    '#options' => $rows,
    '#empty' => t('Collection is empty.'),
    '#js_select' => FALSE,
    '#multiple' => FALSE,
    '#prefix' => $pager,
    '#suffix' => $pager,
  );
}

/**
 * Replaces the thumbnail with one from a selected object
 *
 * @param AbstractObject $object
 *   The paged content object to update.
 *
 * @return bool
 *   TRUE on success, FALSE otherwise.
 */
function islandora_thumbnail_selector_replace_thumbnail(AbstractObject $object, AbstractObject $tn_object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  
  if($tn_object['TN'] == null){
    drupal_set_message(t('The specified object does not have a thumbnail'),'error');
    return;
  }
  
  $mime_detector = new MimeDetect();
  $ext = $mime_detector->getExtension($tn_object['TN']->mimeType);
  // Windows will likely store temp data in a temp directory
  // rather than in memory. Since the colon is forbidden in
  // filenames, replace it with an underscore instead.
  $thumbnail_id = ((islandora_deployed_on_windows()) ? str_replace(':', '_', $tn_object->id) : $tn_object->id);
  $file = drupal_realpath("temporary://{$thumbnail_id}_TN.{$ext}");
  $tn_object['TN']->getContent($file);
  $ds = $object['TN'];
  $ds->setContentFromFile($file, true);
  file_unmanaged_delete($file);
  return true;
}
