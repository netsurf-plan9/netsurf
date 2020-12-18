## Compile the netsurf browser.
#  Resulting executable: 'frontend/framebuffer/$O.nsfb'

</$objtype/mkfile

OBJ=\
	utils/bloom.$O\
	utils/corestrings.$O\
	utils/file.$O\
	utils/filename.$O\
	utils/filepath.$O\
	utils/hashmap.$O\
	utils/hashtable.$O\
	utils/http/cache-control.$O\
	utils/http/challenge.$O\
	utils/http/content-disposition.$O\
	utils/http/content-type.$O\
	utils/http/generics.$O\
	utils/http/parameter.$O\
	utils/http/primitives.$O\
	utils/http/strict-transport-security.$O\
	utils/http/www-authenticate.$O\
	utils/idna.$O\
	utils/libdom.$O\
	utils/log.$O\
	utils/messages.$O\
	utils/nscolour.$O\
	utils/nsoption.$O\
	utils/nsurl/nsurl.$O\
	utils/nsurl/parse.$O\
	utils/punycode.$O\
	utils/ssl_certs.$O\
	utils/talloc.$O\
	utils/time.$O\
	utils/url.$O\
	utils/useragent.$O\
	utils/utf8.$O\
	utils/utils.$O\
	content/content.$O\
	content/content_factory.$O\
	content/fetch.$O\
#	content/fetchers/curl.$O\
	content/fetchers/data.$O\
	content/fetchers/resource.$O\
	content/fetchers/file/dirlist.$O\
	content/fetchers/file/file.$O\
	content/fetchers/about/about.$O\
	content/fetchers/about/blank.$O\
	content/fetchers/about/certificate.$O\
	content/fetchers/about/chart.$O\
	content/fetchers/about/choices.$O\
	content/fetchers/about/config.$O\
	content/fetchers/about/imagecache.$O\
	content/fetchers/about/nscolours.$O\
	content/fetchers/about/query.$O\
	content/fetchers/about/query_auth.$O\
	content/fetchers/about/query_fetcherror.$O\
	content/fetchers/about/query_privacy.$O\
	content/fetchers/about/query_timeout.$O\
	content/fetchers/about/testament.$O\
	content/fs_backing_store.$O\
	content/handlers/css/css.$O\
	content/handlers/css/dump.$O\
	content/handlers/css/hints.$O\
	content/handlers/css/internal.$O\
	content/handlers/css/select.$O\
	content/handlers/css/utils.$O\
	content/handlers/html/box_construct.$O\
	content/handlers/html/box_inspect.$O\
	content/handlers/html/box_manipulate.$O\
	content/handlers/html/box_normalise.$O\
	content/handlers/html/box_special.$O\
	content/handlers/html/box_textarea.$O\
	content/handlers/html/css.$O\
	content/handlers/html/css_fetcher.$O\
	content/handlers/html/dom_event.$O\
	content/handlers/html/font.$O\
	content/handlers/html/form.$O\
	content/handlers/html/forms.$O\
	content/handlers/html/html.$O\
	content/handlers/html/imagemap.$O\
	content/handlers/html/interaction.$O\
	content/handlers/html/layout.$O\
	content/handlers/html/object.$O\
	content/handlers/html/redraw.$O\
	content/handlers/html/redraw_border.$O\
	content/handlers/html/script.$O\
	content/handlers/html/table.$O\
	content/handlers/html/textselection.$O\
	content/handlers/image/bmp.$O\
	content/handlers/image/gif.$O\
	content/handlers/image/ico.$O\
	content/handlers/image/image.$O\
	content/handlers/image/image_cache.$O\
#	content/handlers/image/jpeg.$O\
#	content/handlers/image/nssprite.$O\
#	content/handlers/image/png.$O\
#	content/handlers/image/rsvg.$O\
#	content/handlers/image/svg.$O\
#	content/handlers/image/video.$O\
#	content/handlers/image/webp.$O\
	content/handlers/javascript/content.$O\
	content/handlers/javascript/duktape/dukky.$O\
	content/handlers/javascript/duktape/duktape.$O\
	content/handlers/javascript/fetcher.$O\
	content/handlers/text/textplain.$O\
	content/hlcache.$O\
	content/llcache.$O\
	content/mimesniff.$O\
	content/textsearch.$O\
	content/no_backing_store.$O\
	content/urldb.$O\
	content/webfs.$O\
	desktop/browser.$O\
	desktop/browser_history.$O\
	desktop/browser_window.$O\
	desktop/cookie_manager.$O\
	desktop/cw_helper.$O\
	desktop/download.$O\
	desktop/font_haru.$O\
	desktop/frames.$O\
	desktop/global_history.$O\
	desktop/gui_factory.$O\
	desktop/hotlist.$O\
	desktop/knockout.$O\
	desktop/local_history.$O\
	desktop/mouse.$O\
	desktop/netsurf.$O\
	desktop/page-info.$O\
	desktop/plot_style.$O\
	desktop/print.$O\
#	desktop/save_complete.$O\
	desktop/save_pdf.$O\
	desktop/save_text.$O\
	desktop/scrollbar.$O\
	desktop/search.$O\
	desktop/searchweb.$O\
	desktop/selection.$O\
	desktop/system_colour.$O\
	desktop/textarea.$O\
	desktop/textinput.$O\
	desktop/treeview.$O\
	desktop/version.$O\
	frontends/framebuffer/gui.$O \
	frontends/framebuffer/framebuffer.$O \
	frontends/framebuffer/schedule.$O \
	frontends/framebuffer/bitmap.$O \
	frontends/framebuffer/fetch.$O \
	frontends/framebuffer/findfile.$O \
	frontends/framebuffer/corewindow.$O \
	frontends/framebuffer/local_history.$O \
	frontends/framebuffer/plan9_clipboard.$O \
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
	frontends/framebuffer/image-throbber8.$O\

DUKLIB=content/handlers/javascript/duktape/libduktape.$O.a
DUKOBJ=\
	content/handlers/javascript/duktape/duktape/audio_track.$O\
	content/handlers/javascript/duktape/duktape/audio_track_list.$O\
	content/handlers/javascript/duktape/duktape/application_cache.$O\
	content/handlers/javascript/duktape/duktape/attr.$O\
	content/handlers/javascript/duktape/duktape/autocomplete_error_event.$O\
	content/handlers/javascript/duktape/duktape/autocomplete_error_event_init.$O\
	content/handlers/javascript/duktape/duktape/bar_prop.$O\
	content/handlers/javascript/duktape/duktape/before_unload_event.$O\
	content/handlers/javascript/duktape/duktape/binding.$O\
	content/handlers/javascript/duktape/duktape/broadcast_channel.$O\
	content/handlers/javascript/duktape/duktape/canvas_gradient.$O\
	content/handlers/javascript/duktape/duktape/canvas_pattern.$O\
	content/handlers/javascript/duktape/duktape/canvas_proxy.$O\
	content/handlers/javascript/duktape/duktape/canvas_rendering_context2d.$O\
	content/handlers/javascript/duktape/duktape/canvas_rendering_context2d_settings.$O\
	content/handlers/javascript/duktape/duktape/character_data.$O\
	content/handlers/javascript/duktape/duktape/close_event.$O\
	content/handlers/javascript/duktape/duktape/close_event_init.$O\
	content/handlers/javascript/duktape/duktape/comment.$O\
	content/handlers/javascript/duktape/duktape/composition_event.$O\
	content/handlers/javascript/duktape/duktape/composition_event_init.$O\
	content/handlers/javascript/duktape/duktape/console.$O\
	content/handlers/javascript/duktape/duktape/css.$O\
	content/handlers/javascript/duktape/duktape/css_grouping_rule.$O\
	content/handlers/javascript/duktape/duktape/css_import_rule.$O\
	content/handlers/javascript/duktape/duktape/css_margin_rule.$O\
	content/handlers/javascript/duktape/duktape/css_media_rule.$O\
	content/handlers/javascript/duktape/duktape/css_namespace_rule.$O\
	content/handlers/javascript/duktape/duktape/css_page_rule.$O\
	content/handlers/javascript/duktape/duktape/css_rule.$O\
	content/handlers/javascript/duktape/duktape/css_rule_list.$O\
	content/handlers/javascript/duktape/duktape/css_style_declaration.$O\
	content/handlers/javascript/duktape/duktape/css_style_rule.$O\
	content/handlers/javascript/duktape/duktape/css_style_sheet.$O\
	content/handlers/javascript/duktape/duktape/custom_event.$O\
	content/handlers/javascript/duktape/duktape/custom_event_init.$O\
	content/handlers/javascript/duktape/duktape/data_transfer.$O\
	content/handlers/javascript/duktape/duktape/data_transfer_item.$O\
	content/handlers/javascript/duktape/duktape/data_transfer_item_list.$O\
	content/handlers/javascript/duktape/duktape/dedicated_worker_global_scope.$O\
	content/handlers/javascript/duktape/duktape/document.$O\
	content/handlers/javascript/duktape/duktape/document_fragment.$O\
	content/handlers/javascript/duktape/duktape/document_type.$O\
	content/handlers/javascript/duktape/duktape/dom_element_map.$O\
	content/handlers/javascript/duktape/duktape/dom_implementation.$O\
	content/handlers/javascript/duktape/duktape/dom_parser.$O\
	content/handlers/javascript/duktape/duktape/dom_settable_token_list.$O\
	content/handlers/javascript/duktape/duktape/dom_string_map.$O\
	content/handlers/javascript/duktape/duktape/dom_token_list.$O\
	content/handlers/javascript/duktape/duktape/drag_event.$O\
	content/handlers/javascript/duktape/duktape/drag_event_init.$O\
	content/handlers/javascript/duktape/duktape/drawing_style.$O\
	content/handlers/javascript/duktape/duktape/element.$O\
	content/handlers/javascript/duktape/duktape/error_event.$O\
	content/handlers/javascript/duktape/duktape/error_event_init.$O\
	content/handlers/javascript/duktape/duktape/event.$O\
	content/handlers/javascript/duktape/duktape/event_init.$O\
	content/handlers/javascript/duktape/duktape/event_listener.$O\
	content/handlers/javascript/duktape/duktape/event_modifier_init.$O\
	content/handlers/javascript/duktape/duktape/event_source.$O\
	content/handlers/javascript/duktape/duktape/event_source_init.$O\
	content/handlers/javascript/duktape/duktape/event_target.$O\
	content/handlers/javascript/duktape/duktape/external.$O\
	content/handlers/javascript/duktape/duktape/focus_event.$O\
	content/handlers/javascript/duktape/duktape/focus_event_init.$O\
	content/handlers/javascript/duktape/duktape/hash_change_event.$O\
	content/handlers/javascript/duktape/duktape/hash_change_event_init.$O\
	content/handlers/javascript/duktape/duktape/history.$O\
	content/handlers/javascript/duktape/duktape/hit_region_options.$O\
	content/handlers/javascript/duktape/duktape/html_all_collection.$O\
	content/handlers/javascript/duktape/duktape/html_anchor_element.$O\
	content/handlers/javascript/duktape/duktape/html_applet_element.$O\
	content/handlers/javascript/duktape/duktape/html_area_element.$O\
	content/handlers/javascript/duktape/duktape/html_audio_element.$O\
	content/handlers/javascript/duktape/duktape/html_base_element.$O\
	content/handlers/javascript/duktape/duktape/html_body_element.$O\
	content/handlers/javascript/duktape/duktape/html_br_element.$O\
	content/handlers/javascript/duktape/duktape/html_button_element.$O\
	content/handlers/javascript/duktape/duktape/html_canvas_element.$O\
	content/handlers/javascript/duktape/duktape/html_collection.$O\
	content/handlers/javascript/duktape/duktape/html_data_element.$O\
	content/handlers/javascript/duktape/duktape/html_data_list_element.$O\
	content/handlers/javascript/duktape/duktape/html_details_element.$O\
	content/handlers/javascript/duktape/duktape/html_dialog_element.$O\
	content/handlers/javascript/duktape/duktape/html_directory_element.$O\
	content/handlers/javascript/duktape/duktape/html_div_element.$O\
	content/handlers/javascript/duktape/duktape/html_dlist_element.$O\
	content/handlers/javascript/duktape/duktape/html_element.$O\
	content/handlers/javascript/duktape/duktape/html_embed_element.$O\
	content/handlers/javascript/duktape/duktape/html_field_set_element.$O\
	content/handlers/javascript/duktape/duktape/html_font_element.$O\
	content/handlers/javascript/duktape/duktape/html_form_controls_collection.$O\
	content/handlers/javascript/duktape/duktape/html_form_element.$O\
	content/handlers/javascript/duktape/duktape/html_frame_element.$O\
	content/handlers/javascript/duktape/duktape/html_frame_set_element.$O\
	content/handlers/javascript/duktape/duktape/html_head_element.$O\
	content/handlers/javascript/duktape/duktape/html_heading_element.$O\
	content/handlers/javascript/duktape/duktape/html_hr_element.$O\
	content/handlers/javascript/duktape/duktape/html_html_element.$O\
	content/handlers/javascript/duktape/duktape/html_iframe_element.$O\
	content/handlers/javascript/duktape/duktape/html_image_element.$O\
	content/handlers/javascript/duktape/duktape/html_input_element.$O\
	content/handlers/javascript/duktape/duktape/html_keygen_element.$O\
	content/handlers/javascript/duktape/duktape/html_label_element.$O\
	content/handlers/javascript/duktape/duktape/html_legend_element.$O\
	content/handlers/javascript/duktape/duktape/html_li_element.$O\
	content/handlers/javascript/duktape/duktape/html_link_element.$O\
	content/handlers/javascript/duktape/duktape/html_map_element.$O\
	content/handlers/javascript/duktape/duktape/html_marquee_element.$O\
	content/handlers/javascript/duktape/duktape/html_media_element.$O\
	content/handlers/javascript/duktape/duktape/html_menu_element.$O\
	content/handlers/javascript/duktape/duktape/html_menu_item_element.$O\
	content/handlers/javascript/duktape/duktape/html_meta_element.$O\
	content/handlers/javascript/duktape/duktape/html_meter_element.$O\
	content/handlers/javascript/duktape/duktape/html_mod_element.$O\
	content/handlers/javascript/duktape/duktape/html_object_element.$O\
	content/handlers/javascript/duktape/duktape/html_olist_element.$O\
	content/handlers/javascript/duktape/duktape/html_opt_group_element.$O\
	content/handlers/javascript/duktape/duktape/html_option_element.$O\
	content/handlers/javascript/duktape/duktape/html_options_collection.$O\
	content/handlers/javascript/duktape/duktape/html_output_element.$O\
	content/handlers/javascript/duktape/duktape/html_paragraph_element.$O\
	content/handlers/javascript/duktape/duktape/html_param_element.$O\
	content/handlers/javascript/duktape/duktape/html_picture_element.$O\
	content/handlers/javascript/duktape/duktape/html_pre_element.$O\
	content/handlers/javascript/duktape/duktape/html_progress_element.$O\
	content/handlers/javascript/duktape/duktape/html_quote_element.$O\
	content/handlers/javascript/duktape/duktape/html_script_element.$O\
	content/handlers/javascript/duktape/duktape/html_select_element.$O\
	content/handlers/javascript/duktape/duktape/html_source_element.$O\
	content/handlers/javascript/duktape/duktape/html_span_element.$O\
	content/handlers/javascript/duktape/duktape/html_style_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_caption_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_cell_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_col_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_data_cell_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_header_cell_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_row_element.$O\
	content/handlers/javascript/duktape/duktape/html_table_section_element.$O\
	content/handlers/javascript/duktape/duktape/html_template_element.$O\
	content/handlers/javascript/duktape/duktape/html_text_area_element.$O\
	content/handlers/javascript/duktape/duktape/html_time_element.$O\
	content/handlers/javascript/duktape/duktape/html_title_element.$O\
	content/handlers/javascript/duktape/duktape/html_track_element.$O\
	content/handlers/javascript/duktape/duktape/html_ulist_element.$O\
	content/handlers/javascript/duktape/duktape/html_unknown_element.$O\
	content/handlers/javascript/duktape/duktape/html_video_element.$O\
	content/handlers/javascript/duktape/duktape/image_bitmap.$O\
	content/handlers/javascript/duktape/duktape/image_data.$O\
	content/handlers/javascript/duktape/duktape/keyboard_event.$O\
	content/handlers/javascript/duktape/duktape/keyboard_event_init.$O\
	content/handlers/javascript/duktape/duktape/location.$O\
	content/handlers/javascript/duktape/duktape/media_controller.$O\
	content/handlers/javascript/duktape/duktape/media_error.$O\
	content/handlers/javascript/duktape/duktape/media_list.$O\
	content/handlers/javascript/duktape/duktape/message_channel.$O\
	content/handlers/javascript/duktape/duktape/message_event.$O\
	content/handlers/javascript/duktape/duktape/message_event_init.$O\
	content/handlers/javascript/duktape/duktape/message_port.$O\
	content/handlers/javascript/duktape/duktape/mime_type.$O\
	content/handlers/javascript/duktape/duktape/mime_type_array.$O\
	content/handlers/javascript/duktape/duktape/mouse_event.$O\
	content/handlers/javascript/duktape/duktape/mouse_event_init.$O\
	content/handlers/javascript/duktape/duktape/mutation_event.$O\
	content/handlers/javascript/duktape/duktape/mutation_observer.$O\
	content/handlers/javascript/duktape/duktape/mutation_observer_init.$O\
	content/handlers/javascript/duktape/duktape/mutation_record.$O\
	content/handlers/javascript/duktape/duktape/named_node_map.$O\
	content/handlers/javascript/duktape/duktape/navigator.$O\
	content/handlers/javascript/duktape/duktape/node.$O\
	content/handlers/javascript/duktape/duktape/node_filter.$O\
	content/handlers/javascript/duktape/duktape/node_iterator.$O\
	content/handlers/javascript/duktape/duktape/node_list.$O\
	content/handlers/javascript/duktape/duktape/page_transition_event.$O\
	content/handlers/javascript/duktape/duktape/page_transition_event_init.$O\
	content/handlers/javascript/duktape/duktape/path2d.$O\
	content/handlers/javascript/duktape/duktape/plugin.$O\
	content/handlers/javascript/duktape/duktape/plugin_array.$O\
	content/handlers/javascript/duktape/duktape/pop_state_event.$O\
	content/handlers/javascript/duktape/duktape/pop_state_event_init.$O\
	content/handlers/javascript/duktape/duktape/processing_instruction.$O\
	content/handlers/javascript/duktape/duktape/pseudo_element.$O\
	content/handlers/javascript/duktape/duktape/radio_node_list.$O\
	content/handlers/javascript/duktape/duktape/range.$O\
	content/handlers/javascript/duktape/duktape/related_event.$O\
	content/handlers/javascript/duktape/duktape/related_event_init.$O\
	content/handlers/javascript/duktape/duktape/shared_worker.$O\
	content/handlers/javascript/duktape/duktape/shared_worker_global_scope.$O\
	content/handlers/javascript/duktape/duktape/storage.$O\
	content/handlers/javascript/duktape/duktape/storage_event.$O\
	content/handlers/javascript/duktape/duktape/storage_event_init.$O\
	content/handlers/javascript/duktape/duktape/style_sheet.$O\
	content/handlers/javascript/duktape/duktape/style_sheet_list.$O\
	content/handlers/javascript/duktape/duktape/svg_element.$O\
	content/handlers/javascript/duktape/duktape/text.$O\
	content/handlers/javascript/duktape/duktape/text_metrics.$O\
	content/handlers/javascript/duktape/duktape/text_track.$O\
	content/handlers/javascript/duktape/duktape/text_track_cue.$O\
	content/handlers/javascript/duktape/duktape/text_track_cue_list.$O\
	content/handlers/javascript/duktape/duktape/text_track_list.$O\
	content/handlers/javascript/duktape/duktape/time_ranges.$O\
	content/handlers/javascript/duktape/duktape/touch.$O\
	content/handlers/javascript/duktape/duktape/track_event.$O\
	content/handlers/javascript/duktape/duktape/track_event_init.$O\
	content/handlers/javascript/duktape/duktape/tree_walker.$O\
	content/handlers/javascript/duktape/duktape/ui_event.$O\
	content/handlers/javascript/duktape/duktape/ui_event_init.$O\
	content/handlers/javascript/duktape/duktape/url.$O\
	content/handlers/javascript/duktape/duktape/url_search_params.$O\
	content/handlers/javascript/duktape/duktape/validity_state.$O\
	content/handlers/javascript/duktape/duktape/video_track.$O\
	content/handlers/javascript/duktape/duktape/video_track_list.$O\
	content/handlers/javascript/duktape/duktape/web_socket.$O\
	content/handlers/javascript/duktape/duktape/wheel_event.$O\
	content/handlers/javascript/duktape/duktape/wheel_event_init.$O\
	content/handlers/javascript/duktape/duktape/window.$O\
	content/handlers/javascript/duktape/duktape/worker.$O\
	content/handlers/javascript/duktape/duktape/worker_global_scope.$O\
	content/handlers/javascript/duktape/duktape/worker_location.$O\
	content/handlers/javascript/duktape/duktape/worker_navigator.$O\
	content/handlers/javascript/duktape/duktape/xml_document.$O\
	content/handlers/javascript/duktape/duktape/xml_serializer.$O\

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

CC=pcc

# See to that these are also complied before, along with the libraries
EXTRA=../posix/src/iconv.$O ../posix/src/preadwrite.$O ../posix/src/math9.$O

HFILES=\
	content/handlers/javascript/duktape/duktape/binding.h\
	content/handlers/javascript/duktape/duktape/generics.js.inc\
	content/handlers/javascript/duktape/duktape/polyfill.js.inc

CFLAGS=\
	-B\
	-I . -I include -I ../posix/include \
	-I ../libparserutils/include \
	-I ../libdom/include \
	-I ../libdom/ \
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
	-D_RESEARCH_SOURCE -DHAVE_VA_COPY \
	-DPATH_MAX=_POSIX_PATH_MAX \
	-D_PLAN9 \
	-D __PRETTY_FUNCTION__="func" \
	-D__plan9ape__ \
	-DDUK_F_X64 -DDUK_USE_COMPUTED_INFINITY -DDUK_F_HAVE_INTTYPES \
	-DNETSURF_HOMEPAGE="about:welcome" \
	-DNETSURF_BUILTIN_LOG_FILTER="level:WARNING" \
	-DNETSURF_BUILTIN_VERBOSE_FILTER="level:VERBOSE" \
	-Dnsframebuffer \
	-DNETSURF_FB_RESPATH="/sys/lib/netsurf" \
	-DNETSURF_FB_FONTPATH="/sys/lib/netsurf/fonts" \
	-DWITH_NSLOG -DWITH_BMP -DWITH_GIF \
	-DNETSURF_CONTENT_FETCHERS_FETCH_CURL_H # prevent linking of curl (tmp)

# To add later: -DWITH_CURL

$O.nsfb: $OBJ $LIBS $DUKLIB
	$CC -o $target $OBJ $EXTRA $DUKLIB $LIBS

$DUKLIB: $DUKOBJ $LIBS
	ar rv $DUKLIB $DUKOBJ

%.$O:	%.c $HFILES content/handlers/javascript/duktape/duktape
	$CC $CFLAGS -c -o $target $stem.c

clean:V:
	rm -f $OBJ $DUKOBJ $DUKLIB $O.nsfb

# copy resource files into the directory 9res, that
# is used for install. Run befefore 'mk install'
9res:V:
	dircp resources 9res
	dircp resources/en 9res
	mkdir -p 9res/fonts
	mkdir -p 9res/pointers
	dircp frontends/framebuffer/res/fonts 9res/fonts
	dircp frontends/framebuffer/res/icons 9res/icons
	dircp frontends/framebuffer/res/pointers 9res/pointers

install:V: 9res
	mkdir -p /sys/lib/netsurf
	dircp 9res /sys/lib/netsurf
	cp $O.nsfb /$objtype/bin/nsfb

content/handlers/javascript/duktape/duktape/%.h:Q: content/handlers/javascript/duktape/duktape
	# nothing to do

content/handlers/javascript/duktape/duktape/%.c:Q: content/handlers/javascript/duktape/duktape
	# nothing to do

content/handlers/javascript/duktape/duktape/%.inc:Q: content/handlers/javascript/duktape/duktape
	# nothing to do

content/handlers/javascript/duktape/duktape:
	mkdir -p content/handlers/javascript/duktape/duktape
	dircp ../genfiles/netsurf/duktape content/handlers/javascript/duktape/duktape
