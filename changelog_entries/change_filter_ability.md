 ### WML Engine
   * tag_name is mandatory in [experimental_filter_ability(_active)] but remain optionnal in overwrite_specials[overwrite][experimental_filter_specials]
   * sub_filter [filter_wml] added for check abilities content(subtags or unknow atribute)
   * some attribut to specific were removed('type' for [plague], 'alternative_type' or 'replacement_type' for [damage_type] or overwrite_specials), use [filter_wml] for check these attribute
   * filter 'value' can using "default" value for check if abilities using value by default(explicit 'value' or not)