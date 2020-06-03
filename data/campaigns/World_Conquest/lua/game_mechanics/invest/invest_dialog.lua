local _ = wesnoth.textdomain 'wesnoth-World_Conquest'
local T = wml.tag

local function GUI_FORCE_WIDGET_MINIMUM_SIZE(w,h, content)
	return T.stacked_widget {
		definition = "default",
		T.stack {
			T.layer {
				T.row {
					T.column {
						T.spacer {
							definition = "default",
							width = w,
							height = h
						}
					}
				} 
			},
			T.layer {
				T.row {
					grow_factor = 1,
					T.column {
						grow_factor = 1,
						horizontal_grow = "true",
						vertical_grow = "true",
						content
					}
				} 
			}
		}
	}
end


local dialog_wml = {
	maximum_width = 1200,
	maximum_height = 700,
	T.helptip { id = "tooltip_large" }, -- mandatory field
	T.tooltip { id = "tooltip_large" }, -- mandatory field

	T.linked_group { id = "list_image", fixed_width = true },
	T.linked_group { id = "unit_panel", fixed_width = true },

	T.grid {
		T.row {
			grow_factor = 1,
			T.column {
				border = "all",
				border_size = 5,
				horizontal_alignment = "left",
				T.label {
					definition = "title",
					label = _"Invest",
					id = "title"
				}
			}
		},
		T.row {
			grow_factor = 1,
			T.column {
				horizontal_grow = true,
				vertical_grow = true,
				T.grid {
					T.row {
						T.column {
							border = "all",
							border_size = 5,
							horizontal_grow = true,
							vertical_grow = true,
							T.tree_view {
								id = "left_tree",
								definition = "default",
								horizontal_scrollbar_mode = "never",
								vertical_scrollbar_mode = "initial_auto",
								indentation_step_size = 30,
								T.node {
									id = "category",
									T.node_definition {
										T.row {
											T.column {
												grow_factor = 0,
												horizontal_grow = true,
												T.toggle_button {
													id = "tree_view_node_toggle",
													definition = "tree_view_node",
												},
											},
											T.column {
												grow_factor = 1,
												horizontal_grow = true,
												T.grid {
													T.row {
														T.column {
															horizontal_alignment = "left",
															T.label {
																id = "category_name",
															},
														},
													},
												},
											},
										},
									},
								},
								T.node {
									id = "item_desc",
									T.node_definition {
										T.row {
											T.column {
												grow_factor = 1,
												horizontal_grow = true,
												T.toggle_panel {
													id = "tree_view_node_label",
													T.grid {
														T.row {
															T.column {
																grow_factor = 0,
																horizontal_alignment = "left",
																T.image {
																	id = "image",
																	linked_group = "list_image",
																},
															},
															T.column {
																horizontal_grow = true,
																grow_factor = 1,
																T.grid {
																	T.row {
																		T.column {
																			grow_factor = 1,
																			horizontal_alignment = "left",
																			T.label {
																				id = "name",
																			},
																		},
																	},
																	T.row {
																		T.column {
																			grow_factor = 1,
																			horizontal_alignment = "left",
																			T.label {
																				use_markup = true,
																				id = "desc",
																			},
																		},
																	},
																},
															},
														},
													},
												},
											},
										},
									},
								},
								T.node {
									id = "item",
									T.node_definition {
										T.row {
											T.column {
												grow_factor = 1,
												horizontal_grow = true,
												T.toggle_panel {
													id = "tree_view_node_label",
													T.grid {
														T.row {
															T.column {
																grow_factor = 0,
																T.image {
																	id = "image",
																	linked_group = "list_image",
																},
															},
															T.column {
																horizontal_alignment = "left",
																grow_factor = 1,
																T.grid {
																	T.row {
																		T.column {
																			horizontal_alignment = "left",
																			T.label {
																				id = "name",
																				use_markup = true,
																			},
																		},
																	},
																},
															},
														},
													},
												},
											},
										},
									},
								},
							},
						},
						T.column { vertical_grow = true, T.grid { T.row {
						T.column {
							vertical_grow = true,
							T.multi_page {
								id = "details",
								definition = "default",
								horizontal_scrollbar_mode = "never",
								vertical_grow = true,
								T.page_definition {
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.scroll_label {
												id = "label",
												label = "Text",
												use_markup = true,
											},
										},
									},
								},
								T.page_definition {
									id = "hero",
									T.row {
										T.column {
											vertical_grow = true,
											T.unit_preview_pane {
												definition = "default",
												id = "unit",
												linked_group = "unit_panel",
											} ,
										},
									},
								},
								T.page_definition {
									id = "trailing",
									--T.row { T.column { T.size_lock { width = 600, height = 900, T.widget { T.grid {
									--[[
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.label {
												label = "Before:",
											},
										},
									},
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.scroll_label {
												id = "training_before",
												use_markup = true,
											},
										},
									},
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.label {
												label = "After:",
											},
										},
									},
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.scroll_label {
												id = "training_after",
												use_markup = true,
											},
										},
									},
									--]]
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.scroll_label {
												id = "details",
												use_markup = true,
											},
										},
									},
									--},},},},},
								},
							},
						},
						},
						T.row {
							T.column {
								horizontal_alignment = "center",
								T.button {
									definition = "really_large",
									label = "Get This Item",
									id = "ok",
								}
							}
						}
						},},
					},
				},
			},
		},
		--[[
		T.row {
			T.column {
				horizontal_grow = true,
				T.grid {
					T.row {
						T.column {
							grow_factor = 1,
							T.spacer {
							}
						},
						T.column {
							horizontal_alignment = "right",
							T.button {
								label = "Ok",
								id = "ok",
							}
						}
					}
				}
			}
		}
		--]]
	}
}

return dialog_wml
