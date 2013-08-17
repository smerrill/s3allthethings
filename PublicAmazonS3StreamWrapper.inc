<?php

/**
 * @file
 * Drupal stream wrapper implementation for Amazon S3
 *
 * Implements DrupalStreamWrapperInterface to provide an Amazon S3 wrapper with
 * the s3:// prefix
 */
class PublicAmazonS3StreamWrapper extends AmazonS3StreamWrapper {
  /**
   * Sets the stream resource URI.
   *
   * URIs are formatted as "s3://bucket/key"
   *
   * @return
   *   Returns the current URI of the instance.
   */
  public function setUri($uri) {
    $this->uri = str_replace('public://', '', $uri);
  }

  /**
   * Return the local filesystem path.
   *
   * @param $uri
   *   Optional URI, supplied when doing a move or rename.
   */
  protected function getLocalPath($uri = NULL) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    $path  = str_replace('public://', '', $uri);
    $path  = str_replace('s3://', '', $path);
    $path = trim($path, '/');

    return $path;
  }

  /**
   * Determine a file's mime type.
   * This adds to the parent class by setting them properly for .css.gz and .js.gz.
   */
  public static function getMimeType($uri, $mapping = NULL) {
    if (preg_match('/\.css\.gz/', $uri)) {
      return 'text/css';
    }
    if (preg_match('/\.js\.gz/', $uri)) {
      return 'application/x-javascript';
    }

    return parent::getMimeType($uri, $mapping);
  }

  /**
   * Support for fflush(). Flush current cached stream data to storage.
   *
   * This adds to the parent class by setting the proper Content-Encoding
   * headers for gzipped data.
   *
   * @return
   *   TRUE if data was successfully stored (or there was no data to store).
   *
   * @see http://php.net/manual/en/streamwrapper.stream-flush.php
   */
  public function stream_flush() {
    $content_type = PublicAmazonS3StreamWrapper::getMimeType($this->uri);
    $headers = array(
      'Content-Type' => $content_type,
    );
    if (preg_match('/\.gz(ip)?/', $this->getLocalPath())) {
      $headers['Content-Encoding'] = 'gzip';
    }

    if($this->write_buffer) {
      $response = $this->getS3()->create_object($this->bucket, $this->getLocalPath(), array(
        'body' => $this->buffer,
        'acl' => AmazonS3::ACL_PUBLIC,
        'contentType' => $content_type,
        'headers' => $headers,
      ));
      if($response->isOK()) {
        return TRUE;
      }
    }
    $this->clearBuffer();
    return FALSE;
  }
}
