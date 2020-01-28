## Compile the netsurf browser.
#  Resulting executable: 'frontend/framebuffer/$O.nsfb'

</$objtype/mkfile

CC=pcc
OBJ=\
	utils/bloom.$O \
	utils/corestrings.$O \
	utils/file.$O \
	utils/filename.$O \
	utils/filepath.$O \
	utils/hashtable.$O \
	utils/idna.$O \
	utils/libdom.$O \
	utils/log.$O \
	utils/messages.$O \
	utils/nsoption.$O \
	utils/punycode.$O \
	utils/talloc.$O \
	utils/time.$O \
	utils/url.$O \
	utils/useragent.$O \
	utils/utf8.$O \
	utils/utils.$O \
	utils/nsurl/nsurl.$O \
	utils/nsurl/parse.$O \
	utils/http/challenge.$O \
	utils/http/generics.$O \
	utils/http/primitives.$O \
	utils/http/parameter.$O \
	utils/http/cache-control.$O \
	utils/http/content-disposition.$O \
	utils/http/content-type.$O \
	utils/http/strict-transport-security.$O \
	utils/http/www-authenticate.$O \
	content/content.$O \
	content/content_factory.$O \
	content/dirlist.$O \
	content/fetch.$O \
	content/hlcache.$O \
	content/llcache.$O \
	content/mimesniff.$O \
	content/urldb.$O \
	content/no_backing_store.$O \
	content/fetchers/data.$O \
#	content/fetchers/curl.$O \
	content/fetchers/file.$O \
	content/fetchers/about.$O \
	content/fetchers/resource.$O \
# Content Handlers (maybe not all are needed) \
	content/handlers/image/bmp.$O \
	content/handlers/image/gif.$O \
	content/handlers/image/ico.$O \
	content/handlers/image/image.$O \
	content/handlers/image/image_cache.$O \
#	content/handlers/image/jpeg.$O \
#	content/handlers/image/nssprite.$O \
#	content/handlers/image/png.$O \
#	content/handlers/image/rsvg.$O \
#	content/handlers/image/svg.$O \
#	content/handlers/image/video.$O \
#	content/handlers/image/webp.$O \
	content/handlers/css/css.$O \
	content/handlers/css/dump.$O \
	content/handlers/css/internal.$O \
	content/handlers/css/hints.$O \
	content/handlers/css/select.$O \
	content/handlers/css/utils.$O \
	content/handlers/javascript/fetcher.$O \
	content/handlers/javascript/none/none.$O \
	content/handlers/html/box.$O \
	content/handlers/html/box_construct.$O \
	content/handlers/html/box_normalise.$O \
	content/handlers/html/box_textarea.$O \
	content/handlers/html/font.$O \
	content/handlers/html/form.$O \
	content/handlers/html/imagemap.$O \
	content/handlers/html/layout.$O \
	content/handlers/html/search.$O \
	content/handlers/html/table.$O \
	content/handlers/html/html.$O \
	content/handlers/html/html_css.$O \
	content/handlers/html/html_css_fetcher.$O \
	content/handlers/html/html_script.$O \
	content/handlers/html/html_interaction.$O \
	content/handlers/html/html_redraw.$O \
	content/handlers/html/html_redraw_border.$O \
	content/handlers/html/html_forms.$O \
	content/handlers/html/html_object.$O \
	content/handlers/text/textplain.$O \
	desktop/cookie_manager.$O \
	desktop/knockout.$O \
	desktop/hotlist.$O \
	desktop/mouse.$O \
	desktop/plot_style.$O \
	desktop/print.$O \
	desktop/search.$O \
	desktop/searchweb.$O \
	desktop/scrollbar.$O \
	desktop/sslcert_viewer.$O \
	desktop/textarea.$O \
	desktop/version.$O \
	desktop/system_colour.$O \
	desktop/local_history.$O \
	desktop/global_history.$O \
	desktop/treeview.$O \
	desktop/browser.$O \
	desktop/browser_history.$O \
	desktop/download.$O \
	desktop/frames.$O \
	desktop/netsurf.$O \
#	desktop/save_complete.$O \
	desktop/save_text.$O \
	desktop/selection.$O \
	desktop/textinput.$O \
	desktop/gui_factory.$O \
	desktop/save_pdf.$O \
	desktop/font_haru.$O \
	frontends/framebuffer/gui.$O \
	frontends/framebuffer/framebuffer.$O \
	frontends/framebuffer/schedule.$O \
	frontends/framebuffer/bitmap.$O \
	frontends/framebuffer/fetch.$O \
	frontends/framebuffer/findfile.$O \
	frontends/framebuffer/corewindow.$O \
	frontends/framebuffer/local_history.$O \
	frontends/framebuffer/clipboard.$O \
	frontends/framebuffer/fbtk/fbtk.$O \
	frontends/framebuffer/fbtk/event.$O \
	frontends/framebuffer/fbtk/fill.$O \
	frontends/framebuffer/fbtk/bitmap.$O \
	frontends/framebuffer/fbtk/user.$O \
	frontends/framebuffer/fbtk/window.$O \
	frontends/framebuffer/fbtk/text.$O \
	frontends/framebuffer/fbtk/scroll.$O \
	frontends/framebuffer/fbtk/osk.$O \
	frontends/framebuffer/font_internal.$O \
	frontends/framebuffer/font-ns-sans.$O \
	frontends/framebuffer/image-caret_image.$O \
	frontends/framebuffer/image-hand_image.$O \
	frontends/framebuffer/image-history_image.$O \
	frontends/framebuffer/image-history_image_g.$O \
	frontends/framebuffer/image-left_arrow.$O \
	frontends/framebuffer/image-left_arrow_g.$O \
	frontends/framebuffer/image-menu_image.$O \
	frontends/framebuffer/image-move_image.$O \
	frontends/framebuffer/image-osk_image.$O \
	frontends/framebuffer/image-pointer_image.$O \
	frontends/framebuffer/image-progress_image.$O \
	frontends/framebuffer/image-reload.$O \
	frontends/framebuffer/image-reload_g.$O \
	frontends/framebuffer/image-right_arrow.$O \
	frontends/framebuffer/image-right_arrow_g.$O \
	frontends/framebuffer/image-scrolld.$O \
	frontends/framebuffer/image-scrolll.$O \
	frontends/framebuffer/image-scrollr.$O \
	frontends/framebuffer/image-scrollu.$O \
	frontends/framebuffer/image-stop_image.$O \
	frontends/framebuffer/image-stop_image_g.$O \
	frontends/framebuffer/image-throbber0.$O \
	frontends/framebuffer/image-throbber1.$O \
	frontends/framebuffer/image-throbber2.$O \
	frontends/framebuffer/image-throbber3.$O \
	frontends/framebuffer/image-throbber4.$O \
	frontends/framebuffer/image-throbber5.$O \
	frontends/framebuffer/image-throbber6.$O \
	frontends/framebuffer/image-throbber7.$O \
	frontends/framebuffer/image-throbber8.$O


# Be sure to compile the libraries first
LIBS=\
	../libnsfb/src/libnsfb.$O.a \
	../libnspsl/src/libnspsl.$O.a \
	../libdom/src/libdom.$O.a \
	../libcss/src/libcss.$O.a \
	../libhubbub/src/libhubbub.$O.a \
	../libparserutils/src/libparserutils.$O.a \
	../libwapcaplet/src/libwapcaplet.$O.a \
	../libnsutils/src/libnsutils.$O.a \
	../libnslog/src/libnslog.$O.a \
	../libnsbmp/src/libnsbmp.$O.a \
	../libnsgif/src/libnsgif.$O.a \
	../libutf8proc/src/libutf8proc.$O.a \

# See to that these are also complied before, along with the libraries
EXTRA=../posix/src/iconv.$O ../posix/src/preadwrite.$O ../posix/src/math9.$O

HFILES=

CFLAGS=\
	-I . -I include -I ../posix/include \
	-I ../libparserutils/include \
	-I ../libdom/include \
	-I ../libwapcaplet/include \
	-I ../libutf8proc/include \
	-I ../libhubbub/include \
	-I ../libnslog/include \
	-I ../libnsutils/include \
	-I ../libnsbmp/include \
	-I ../libnsgif/include \
	-I ../libcss/include \
	-I ../libnsfb/include \
	-I libinclude \
	-I content/handlers \
	-I frontends \
	-D_BSD_EXTENSION -D_SUSV2_SOURCE -D_POSIX_SOURCE \
	-D_C99_SNPRINTF_EXTENSION \
	-DPATH_MAX=_POSIX_PATH_MAX \
	-D__plan9ape__ \
	-DNETSURF_HOMEPAGE="about:welcome" \
	-DNETSURF_BUILTIN_LOG_FILTER="level:WARNING" \
	-DNETSURF_BUILTIN_VERBOSE_FILTER="level:VERBOSE" \
	-Dnsframebuffer \
	-DNETSURF_FB_RESPATH="/sys/lib/netsurf" \
	-DNETSURF_FB_FONTPATH="/sys/lib/netsurf/fonts" \
	-DWITH_NSLOG \
	-DNETSURF_CONTENT_FETCHERS_FETCH_CURL_H # prevent linking of curl (tmp)

# To add later: -DWITH_CURL

$O.nsfb:	$OBJ $LIBS
	$CC -o $target $OBJ $EXTRA $LIBS


%.$O:	%.c $HFILES
	$CC $CFLAGS -c -o $target $stem.c

clean:V:
	rm -f $OBJ $O.nsfb



