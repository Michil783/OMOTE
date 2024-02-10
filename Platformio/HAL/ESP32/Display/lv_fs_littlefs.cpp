#include <FS.h>
#include <LittleFS.h>
#include <lvgl.h>
#include <esp_debug_helpers.h>

File f;
static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
  LV_LOG_TRACE("fs_open(%s, %d)", path, mode);

  LV_UNUSED(drv);
 
  const char * flags = "";
 
  if (mode == LV_FS_MODE_WR) flags = FILE_WRITE;
  else if (mode == LV_FS_MODE_RD) flags = FILE_READ;
  else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) flags = FILE_WRITE;
 
  char complete_path[strlen(path) + 1];
  complete_path[0] = '/';
  complete_path[1] = '\0';
  strcat(complete_path, path);
  LV_LOG_TRACE("Try to open file: %s", complete_path);
  f = LittleFS.open(complete_path, flags);
  if (!f) {
    LV_LOG_ERROR("Error with file: %s", complete_path);
  } else {
    time_t t = f.getLastWrite();
    struct tm * tmstruct = localtime(&t);
    LV_LOG_INFO("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
  }
 
  return (void*)(&f);
}
 
/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p)
{
  LV_LOG_TRACE("fs_close(%s)", ((File*)file_p)->name());
  LV_UNUSED(drv);
  File* file = (File*)file_p;
  file->close();
  return LV_FS_RES_OK;
}
 
/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
  LV_UNUSED(drv);
  File* file = (File*)file_p;
  *br = file->read((uint8_t*)buf, btr);
  LV_LOG_TRACE("fs_read(%s, %p, bytes to read %d, bytes read %d)", ((File*)file_p)->name(), buf, btr, *br);
  /*
  if( *br ){
    Serial.print("buffer: ");
    for(int i = 0; i < *br; i++){
        Serial.printf("%0.2X", ((uint8_t*)buf)[i]);
        if( i < *br ) Serial.print(",");
    }
    Serial.println("");
  }
  */
  return (int32_t)(*br) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}
 
/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
  LV_UNUSED(drv);
  File* file = (File*)file_p;
  *bw = file->write((uint8_t*)buf, btw);
  LV_LOG_TRACE("fs_write(%s, %p, bytes to write %d, bytes written %d)", ((File*)file_p)->name(), buf, btw, *bw);
  return (int32_t)(*bw) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}
 
/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
  LV_LOG_TRACE("fs_seek(%s, pos %d, from %d)", ((File*)file_p)->name(), pos, whence);
  LV_UNUSED(drv);
  SeekMode mode;
  if (whence == LV_FS_SEEK_SET) mode = SeekSet;
  else if (whence == LV_FS_SEEK_CUR) mode = SeekCur;
  else if (whence == LV_FS_SEEK_END) mode = SeekEnd;
 
  File* file = (File*)file_p;
  file->seek(pos, mode);
  return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos){
    File* file = (File*)file_p;
    *pos = file->position();
    LV_LOG_TRACE("%s, %d", ((File*)file_p)->name(), *pos);
    return LV_FS_RES_OK;
}

void lv_port_littlefs_init(void)
{
  LV_LOG_TRACE("lv_port_littlefs_init()");
  /*Add a simple drive to open images*/
  static lv_fs_drv_t fs_drv;
  lv_fs_drv_init(&fs_drv);
 
  fs_drv.letter = 'L';
  fs_drv.open_cb = fs_open;
  fs_drv.close_cb = fs_close;
  fs_drv.read_cb = fs_read;
  fs_drv.write_cb = fs_write;
  fs_drv.seek_cb = fs_seek;
  fs_drv.tell_cb = fs_tell;
 
  fs_drv.dir_close_cb = NULL;
  fs_drv.dir_open_cb = NULL;
  fs_drv.dir_read_cb = NULL;
 
  lv_fs_drv_register(&fs_drv);
}

#ifndef LV_USE_PNG
/*Create a new decoder and register functions */
lv_img_decoder_t * dec = lv_img_decoder_create();
lv_img_decoder_set_info_cb(dec, decoder_info);
lv_img_decoder_set_open_cb(dec, decoder_open);
lv_img_decoder_set_close_cb(dec, decoder_close);

/**
 * Get info about a PNG image
 * @param decoder pointer to the decoder where this function belongs
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t decoder_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header)
{
  /*Check whether the type `src` is known by the decoder*/
  if(is_png(src) == false) return LV_RES_INV;

  /* Read the PNG header and find `width` and `height` */
  ...

  header->cf = LV_IMG_CF_RAW_ALPHA;
  header->w = width;
  header->h = height;
}

/**
 * Open a PNG image and return the decided image
 * @param decoder pointer to the decoder where this function belongs
 * @param dsc pointer to a descriptor which describes this decoding session
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t decoder_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{

  /*Check whether the type `src` is known by the decoder*/
  if(is_png(src) == false) return LV_RES_INV;

  /*Decode and store the image. If `dsc->img_data` is `NULL`, the `read_line` function will be called to get the image data line-by-line*/
  dsc->img_data = my_png_decoder(src);

  /*Change the color format if required. For PNG usually 'Raw' is fine*/
  dsc->header.cf = LV_IMG_CF_...

  /*Call a built in decoder function if required. It's not required if`my_png_decoder` opened the image in true color format.*/
  lv_res_t res = lv_img_decoder_built_in_open(decoder, dsc);

  return res;
}

/**
 * Decode `len` pixels starting from the given `x`, `y` coordinates and store them in `buf`.
 * Required only if the "open" function can't open the whole decoded pixel array. (dsc->img_data == NULL)
 * @param decoder pointer to the decoder the function associated with
 * @param dsc pointer to decoder descriptor
 * @param x start x coordinate
 * @param y start y coordinate
 * @param len number of pixels to decode
 * @param buf a buffer to store the decoded pixels
 * @return LV_RES_OK: ok; LV_RES_INV: failed
 */
lv_res_t decoder_built_in_read_line(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc, lv_coord_t x,
                                                  lv_coord_t y, lv_coord_t len, uint8_t * buf)
{
   /*With PNG it's usually not required*/

   /*Copy `len` pixels from `x` and `y` coordinates in True color format to `buf` */

}

/**
 * Free the allocated resources
 * @param decoder pointer to the decoder where this function belongs
 * @param dsc pointer to a descriptor which describes this decoding session
 */
static void decoder_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{
  /*Free all allocated data*/

  /*Call the built-in close function if the built-in open/read_line was used*/
  lv_img_decoder_built_in_close(decoder, dsc);

}
#endif