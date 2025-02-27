---@meta

gui = {}

---Definition of a menu item for gui.show_menu
---@class gui_menu_item
---@field icon string
---@field image string
---@field label tstring
---@field details tstring
---@field tooltip tstring

---Show a popup menu at the current mouse locations
---@param items gui_menu_item[]
---@param initial? integer Specifies the index of the item that should be selected when the menu opens.
---@param markup? boolean Whether to parse Pango markup. Default false.
---@return string #The index of the selected item, or 0 if cancelled by clicking outside the menu's boundary
function gui.show_menu(items, initial, markup) end

---@class gui_narration_params
---@field title tstring
---@field message tstring
---@field portrait string
---@field left_side boolean
---@field mirror boolean
---@class gui_narration_option_info
---@field default boolean
---@field image string
---@field label tstring
---@field description tstring
---@class gui_narration_text_params
---@field label tstring
---@field text string
---@field max_length integer

---Shows a message dialog.
---@param attributes gui_narration_params
---@param options? string[]|gui_narration_option_info[]
---@param text_input? gui_narration_text_params
---@return integer
---| '-2' #User pressed Escape to close the message
---| '-1' #User pressed Space or clicked the mouse to close the message
---| '0' #If there is text input but no options
---| '1+' #The index of the selected option
---@return string
function gui.show_narration(attributes, options, text_input) end

---Show a simple popup dialog in the centre of the screen
---@param title tstring A title string for the dialog
---@param message tstring The message to show
---@param image? string An image to show
function gui.show_popup(title, message, image) end

---Show a simple prompt dialog with one or two buttons.
---@alias gui_prompt_button_type
---| "'ok'" #A single OK button
---| "'cancel'" #A single Cancel button
---| "'close'" #A single Close button
---| "'ok_cancel'" #Two buttons labelled OK and Cancel
---| "'yes_no'" #Two buttons labelled Yes and No
---@param title tstring A title string for the dialog
---@param message tstring The message to show
---@param button? string|gui_prompt_button_type The button label
---@param markup? boolean Whether to parse Pango markup
---@return boolean #false if No or Cancel was clicked, otherwise true
function gui.show_prompt(title, message, button, markup) end

---Shows the storyscreen
---@param story WML
---@param title tstring
function gui.show_story(story, title) end

---Open the in-game help
---@param topic? string
function gui.show_help(topic) end

---Open the gamestate inspector
---@param title? string A title to be displayed in the inspector. Can be used to disambiguate where it was called from.
function gui.show_inspector(title) end

---Show the recruit dialog with the given list of unit types and configuration
---@class unit_dialog_options
---@field title? string the title of the dialog
---@field ok_label? string caption of the button with id "ok"
---@field cancel_label? string caption of the button with id "cancel"
---@field help_topic? string the help topic that is to be opened when the '?' button is clicked

---@param unit_types_list unit_type[] list of units types to be shown as available recruits
---@param options? unit_dialog_options a list of optional configuration keys.
---@return unit_type the selected unit type or nil if not selected
function gui.show_recruit_dialog(unit_types_list, options) end

---Show the recall dialog with the given list of units and configuration
---@param units_list unit[] list of units types to be shown as available recalls
---@param options? unit_dialog_options a list of optional configuration keys.
---@return unit the selected unit or nil if not selected
function gui.show_recall_dialog(units_list, options) end

---Open the in-game Lua console
function gui.show_lua_console() end

-- Show a custom dialog
---@param cfg WML The dialog's [resolution] WML
---@param preshow? fun(window:window)
---@param postshow? fun(window:window)
---@return integer
function gui.show_dialog(cfg, preshow, postshow) end

---Changes the current ui (gui2) theme
---@param theme_id string The id of the gui2 theme to switch to
function gui.switch_theme(theme_id) end

-- Add a custom widget definition, for use in a custom dialog
---@param type string The type of widget the definition applies to
---@param id string An ID for the definition, to be referenced from WML
---@param content WML The definition WML
function gui.add_widget_definition(type, id, content) end

---A reference to a widget in a custom dialog box
---@class widget : gui.widget
---@field enabled boolean
---@field help tstring
---@field tooltip tstring
---@field visible boolean|"'visible'"|"'hidden'"|"'invisible'"
---@field type string
---@field text_alignment "'left'"|"'right'"|"'center'"
---@field ellipsize_mode "'none'"|"'start'"|"'middle'"|"'end'"
---@field overflow_to_tooltip boolean
---@field on_left_click fun()

---The window widget is a container that contains all other widgets in the dialog
---@class window : widget

---A simple widget, with no children
---@class simple_widget : widget
---@field use_markup boolean
---@field label tstring|string|number
---@field marked_up_text tstring

---A button with two or more states, typically used as a checkbox or radiobutton
---@class toggle_button : simple_widget
---@field selected boolean
---@field selected_index integer
---@field on_modified fun()

---Similar to a toggle button, but allows for arbitrary contents
---@class toggle_panel : widget
---@field selected boolean
---@field selected_index integer
---@field on_modified fun()

---A dynamic list of items, shown with a scrollbar
---@class listbox : widget
---@field selected_index integer
---@field item_count integer
---@field on_modified fun()

---A container widget that shows only one of its children at any given time
---@class multi_page : widget
---@field selected_index integer
---@field item_count integer

---A container widget whose children all occupy the same space, overlayed on top of each other
---@class stacked_widget : widget
---@field selected_index integer
---@field item_count integer

---A button that produces a dropdown menu when clicked
---@class menu_button : widget
---@field selected_index integer
---@field on_modified fun()

---A button that produces a dropdown menu when clicked in addition to supporting text input
---@class combobox : widget
---@field hint_image string
---@field hint_text tstring
---@field item_count integer
---@field max_input_length integer
---@field selected_index integer
---@field on_modified fun()

---An editable text box
---@class text_box : widget
---@field text string
---@field editable boolean
---@field hint_image string
---@field hint_text tstring
---@field history string
---@field max_input_length integer
---@field on_modified fun()

---A label that wraps its text and also has a vertical scrollbar
---@class scroll_label : widget
---@field link_aware boolean

---A multiline text area that shows a scrollbar if the text gets too long
---@class scroll_text : widget
---@field link_aware boolean

---A slider
---@class slider : widget
---@field value integer
---@field min_value integer
---@field max_value integer
---@field best_slider_length integer
---@field maximum_value_label tstring
---@field minimum_value_label tstring
---@field step_size integer
---@field on_modified fun()

---A progress bar
---@class progress_bar : widget
---@field percentage number

---A dynamic, hierarchical list of items, shown with a scrollbar
---@class treeview : widget
---@field selected_item_path integer[]
---@field item_count integer
---@field unfolded boolean
---@field on_modified fun()

---A single node in a tree view
---@class tree_view_node : widget
---@field path integer[]
---@field item_count integer
---@field unfolded boolean

---A panel that shows details on a given unit or unit type
---@class unit_preview_pane : widget
---@field unit unit|unit_type

---A simple button that triggers once when clicked
---@class button : simple_widget
---@field on_button_click fun()

---A static text label
---@class label : simple_widget
---@field characters_per_line integer
---@field link_aware boolean
---@field link_color string
---@field wrap boolean

---A label that shows formatted text marked up with Help markup
---@class rich_label : simple_widget
---@field link_color string
---@field wrap boolean
---@field on_link_click fun(dest:string)

---A simple image
---@class image : simple_widget
---A simple button that triggers repeatedly if the mouse is held down
---@class repeating_button : button
---A generic container that lays out its children in a grid
---@class grid : widget

---@class gui.widget
gui.widget = {}

---Switch keyboard focus to the widget
---@param widget widget
function gui.widget.focus(widget) end

---Set the canvas definition of a widget
---@param widget widget
---@param layer integer
---@param content WML
function gui.widget.set_canvas(widget, layer, content) end

---Add an item to a homogenous container widget
---@param widget widget
---@param position? integer
---@return widget
---@return integer
function gui.widget.add_item(widget, position) end

---Add an item to a heterogenous container widget
---@param widget widget
---@param category string
---@param position? integer
---@param count? integer
---@return widget
---@return integer
function gui.widget.add_item_of_type(widget, category, position, count) end

---Remove items from a container widget
---@param widget widget
---@param position? integer
function gui.widget.remove_items_at(widget, position) end

---Remove all items from a container widget
---@param widget widget
function gui.widget.clear_items(widget) end

---Find a widget based on a path
---@param widget widget
---@vararg integer|string
function gui.widget.find(widget, ...) end

---Close the dialog
---@param window window
function gui.widget.close(window) end
