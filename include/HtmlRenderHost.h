#ifndef HTMLRENDERHOST_H
#define HTMLRENDERHOST_H

#include <litehtml.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

class HtmlRenderHost : public litehtml::document_container {
public:
		HtmlRenderHost() = default;
		HtmlRenderHost(SDL_Renderer* renderer);

		//This is here to delete our copy constructor for when we start handling resources
		//We don't want to be accidently copying pointers that won't be cleaned up.
		HtmlRenderHost(const HtmlRenderHost&) = delete;
		HtmlRenderHost& operator=(const HtmlRenderHost&) = delete;

		litehtml::uint_ptr	create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override;
		void				delete_font(litehtml::uint_ptr hFont) override;
		int					text_width(const char* text, litehtml::uint_ptr hFont) override;
		void				draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
		int					pt_to_px(int pt) const override;
		int					get_default_font_size() const override;
		const char*			get_default_font_name() const override;
		void				draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
        void				load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
		void				get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override;
		void				draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) {};
		void				draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override;
        void				get_media_features(litehtml::media_features& media) const override;
		void				get_viewport(litehtml::position& viewport) const override;
		void				draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) {}
		void				draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) {}
		void				draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) {}
		void				draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
		void				set_caption(const char* caption) {}
		void				set_base_url(const char* base_url) {}
		void				link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) {}
		void				on_anchor_click(const char* url, const litehtml::element::ptr& el) {}
		bool				on_element_click(const litehtml::element::ptr& /*el*/) { return false; }
		void				on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) {}
		void				set_cursor(const char* cursor) {}
		void				transform_text(litehtml::string& text, litehtml::text_transform tt) override;
		void				import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
		void				set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) {}
		void				del_clip() {}
		litehtml::element::ptr	create_element( const char* tag_name,
														const litehtml::string_map& attributes,
														const std::shared_ptr<litehtml::document>& doc) override;
		void				get_language(litehtml::string& language, litehtml::string& culture) const override;
		litehtml::string	resolve_color(const litehtml::string& /*color*/) const { return litehtml::string(); }
		void				split_text(const char* text, const std::function<void(const char*)>& on_word, const std::function<void(const char*)>& on_space) override;

		// Non-override methods
		void set_renderer(SDL_Renderer* renderer);
private:
    SDL_Renderer* mRenderer = nullptr;
};

#endif