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

local GUI_HORIZONTAL_SPACER_LINE = T.row {
	grow_factor = 0,
	T.column {
		grow_factor = 1,
		border = "all",
		border_size = 10,
		horizontal_grow = true,
		T.drawing {
			definition = "default",
			width = "(width)",
			height = 1,
			T.draw {
				T.line {
					x1 = 0,
					y1 = 0,
					x2 = "(width - 1)",
					y2 = 0,
					color = "114, 79, 46, 255",
					thickness = 1,
				}
			}
		}
	}
}

return {
	maximum_width = 900,
	T.helptip { id = "tooltip_large" }, -- mandatory field
	T.tooltip { id = "tooltip_large" }, -- mandatory field
		
	T.linked_group { id = "artifact_icon", fixed_width = true },
	T.linked_group { id = "artifact_name", fixed_width = true },
	T.linked_group { id = "recruit_icon", fixed_width = true },
	T.linked_group { id = "recruit_name", fixed_width = true },

	T.grid {
		T.row {
			grow_factor = 1,
			T.column {
				border = "all",
				border_size = 5,
				horizontal_alignment = "left",
				T.label {
					definition = "title",
					label = _"WoCopedia",
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
								vertical_scrollbar_mode = "never",
								indentation_step_size = 35,
								T.node {
									id = "training_category",
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
																horizontal_alignment = "left",
																T.label {
																	id = "training_name",
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
												T.toggle_panel {
													id = "tree_view_node_label",
													T.grid {
														T.row {
															T.column {
																horizontal_alignment = "left",
																T.label {
																	id = "training_name",
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
						T.column {
							T.multi_page {
								id = "details",
								definition = "default",
								horizontal_scrollbar_mode = "never",
								T.page_definition {
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.scrollbar_panel { 
												T.definition { 
													T.row { 
														T.column {
															horizontal_grow = true,
															vertical_grow = true,
															T.tree_view {
																id = "tree_details",
																definition = "default",
																horizontal_scrollbar_mode = "never",
																vertical_scrollbar_mode = "never",
																indention_step_size = 20,
																T.node {
																	id = "training_details",
																	T.node_definition {
																		T.row {
																			T.column {
																				grow_factor = 1,
																				horizontal_alignment = "left",
																				T.label {
																					definition = "default_large",
																					id = "training_caption",
																					label = "default description",
																				},
																			},
																		},
																		T.row {
																			T.column {
																				grow_factor = 1,
																				horizontal_alignment = "left",
																				T.label {
																					id = "training_description",
																					label = "default description",
																				},
																			},
																		},
																	},
																},
																T.node {
																	id = "scroll_details",
																	T.node_definition {
																		T.row {
																			T.column {
																				grow_factor = 1,
																				horizontal_alignment = "left",
																				T.label {
																					definition = "default_large",
																					id = "training_caption",
																					label = "default description",
																				},
																			},
																		},
																		T.row {
																			T.column {
																				grow_factor = 1,
																				horizontal_alignment = "left",
																				T.scroll_label {
																					id = "training_description",
																					label = "default description",
																				},
																			},
																		},
																	},
																},
																T.node {
																	id = "seperator",
																	T.node_definition {
																		T.row {
																			T.column {
																				grow_factor = 1,
																				horizontal_grow = true,
																				T.spacer {
																					height = 20,
																					width = 20,
																				},
																			},
																		},
																	},
																},
															} 
														} 
													}
												} 
											} 
										}
									}
								},
								T.page_definition {
									id="simpletext",
									T.row {
										T.column {
											horizontal_grow = true,
											vertical_grow = true,
											T.scroll_label {
												id = "label",
												label = "Text",
											} 
										}
									}
								},
								T.page_definition {
									id="artifact_list",
									vertical_grow = true,
									T.row {
										T.column {
											vertical_grow = true,
											T.grid {
												--T.row {
												--	T.column {
												--		grow_factor = 1,
												--		vertical_grow = true,
												--		T.label {
												--			id = "title",
												--			label = "Artifacts",
												--		}
												--	}
												--},
												T.row {
													T.column {
														T.scroll_label {
															vertical_scrollbar_mode = "never",
															id = "desc",
														}
													}
												},
												T.row {
													T.column {
														horizontal_grow = true,
														T.tree_view {
															vertical_grow = true,
															id = "artifact_list_tv",
															definition = "default",
															horizontal_scrollbar_mode = "never",
															vertical_scrollbar_mode = "always",
															indentation_step_size = 35,
															T.node {
																id = "artifact",
																horizontal_grow = true,
																T.node_definition {
																	vertical_grow = true,
																	T.row {
																		horizontal_grow = true,
																		T.column {
																			horizontal_grow = true,
																			T.grid {
																				T.row {
																					T.column {
																						T.image {
																							id="image",
																							linked_group = "artifact_icon",
																						}
																					},
																					T.column {
																						T.label {
																							id="label",
																							linked_group = "artifact_name",
																						}
																					},
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												},
												T.row {
													grow_factor = 1,
													T.column {
														grow_factor = 0,
														vertical_grow = true,
														T.spacer {
															id = "aa",
														}
													}
												},
											}
										}
									}
								},
								T.page_definition {
									id="faction_info",
									vertical_grow = true,
									T.row {
										T.column {
											vertical_grow = true,
											horizontal_grow = true,
											T.grid {
												--T.row {
												--	T.column {
												--		grow_factor = 1,
												--		vertical_grow = true,
												--		T.label {
												--			id = "title",
												--			label = "Artifacts",
												--		}
												--	}
												--},
												T.row {
													T.column {
														T.scroll_label {
															vertical_scrollbar_mode = "never",
															id = "desc",
														}
													}
												},
												T.row {
													T.column {
														T.label {
															vertical_scrollbar_mode = "never",
															label = "<b>Recruits:</b>",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														horizontal_grow = true,
														T.tree_view {
															id = "recruit_pairs",
															definition = "default",
															horizontal_scrollbar_mode = "never",
															vertical_scrollbar_mode = "always",
															indentation_step_size = 35,
															T.node {
																id = "recruit_pair",
																horizontal_grow = true,
																T.node_definition {
																	T.row {
																		T.column {
																			horizontal_grow = true,
																			T.grid {
																				T.row {
																					T.column {
																						T.image {
																							id="image1",
																							linked_group = "recruit_icon",
																						}
																					},
																					T.column {
																						T.label {
																							id="label1",
																							linked_group = "recruit_name",
																						}
																					},
																					T.column {
																						T.label {
																							label="↔",
																						}
																					},
																					T.column {
																						T.image {
																							id="image2",
																							linked_group = "recruit_icon",
																						}
																					},
																					T.column {
																						T.label {
																							id="label2",
																							linked_group = "recruit_name",
																						}
																					},
																				}
																			}
																		}
																	}
																}
															}
														}
													}
												},
												T.row {
													T.column {
														T.label {
															vertical_scrollbar_mode = "never",
															label = "<b>Deserters:</b>",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														horizontal_grow=true,
														T.scroll_label {
															vertical_scrollbar_mode = "never",
															id = "deserters",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														T.label {
															vertical_scrollbar_mode = "never",
															label = "<b>Commanders:</b>",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														horizontal_grow=true,
														T.scroll_label {
															vertical_scrollbar_mode = "never",
															id = "commanders",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														T.label {
															vertical_scrollbar_mode = "never",
															label = "<b>Heroes:</b>",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														horizontal_grow=true,
														T.scroll_label {
															vertical_scrollbar_mode = "never",
															id = "heroes",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														T.label {
															vertical_scrollbar_mode = "never",
															label = "<b>Random Leaders:</b>",
															id = "tit_random_leaders",
															use_markup = true,
														}
													}
												},
												T.row {
													T.column {
														horizontal_grow=true,
														T.scroll_label {
															vertical_scrollbar_mode = "never",
															id = "random_leaders",
															use_markup = true,
														}
													}
												},
												T.row {
													grow_factor = 1,
													T.column {
														grow_factor = 0,
														vertical_grow = true,
														T.spacer {
															id = "aa",
														}
													}
												},
											}
										}
									}
								},
								T.page_definition {
									id="settings",
									vertical_grow = true,
									horizontal_grow = true,
									T.row {
										T.column {
											vertical_grow = true,
											horizontal_grow = true,
											T.grid {
												T.row {
													grow_factor = 0,
													T.column {
														grow_factor = 0,
														vertical_grow = false,
														horizontal_alignment = "left",
														T.label {
															id = "title",
															label = "<b>Settings</b>\n ",
															use_markup=true,
														},
													},
												},
												T.row {
													grow_factor = 0,
													T.column {
														horizontal_grow=true,
														vertical_alignment="top",
														grow_factor = 1,
														T.grid {
															T.row {
																T.column {
																	horizontal_alignment="left",
																	T.label {
																		label = "Use WC2 PYA mod",
																	}
																},
																T.column {
																	horizontal_alignment="right",
																	T.toggle_button {
																		label = "",
																		id="checkbox_use_pya",
																	},
																},
															},
															T.row {
																T.column {
																	horizontal_alignment="left",
																	T.label {
																		label = "Use WC2 Unitmarkers",
																	}
																},
																T.column {
																	horizontal_alignment="right",
																	T.toggle_button {
																		label = "",
																		id="checkbox_use_markers",
																	},
																},
															},
															T.row {
																T.column {
																	horizontal_alignment="left",
																	T.label {
																		label = "Use WC2 experimental pickup",
																	}
																},
																T.column {
																	horizontal_alignment="right",
																	T.toggle_button {
																		label = "",
																		id="checkbox_use_pickup",
																	},
																},
															},
															T.row {
																T.column {
																	horizontal_alignment="left",
																	T.label {
																		label = "Show pickup confirmation",
																	}
																},
																T.column {
																	horizontal_alignment="right",
																	T.toggle_button {
																		label = "",
																		id="checkbox_show_pickup_confirmation",
																	},
																},
															},
															T.row {
																T.column {
																	horizontal_alignment="left",
																	T.label {
																		label = "difficulty",
																	}
																},
																T.column {
																	horizontal_alignment="right",
																	T.label {
																		label = "",
																		id="label_difficulty",
																	},
																},
															},
															T.row {
																T.column {
																	horizontal_alignment="left",
																	T.label {
																		label = "WC2 version",
																	}
																},
																T.column {
																	horizontal_alignment="right",
																	T.label {
																		label = "",
																		id="label_version",
																	},
																},
															},
														}
													}
												},
												GUI_HORIZONTAL_SPACER_LINE,
												T.row {
													grow_factor = 0,
													T.column {
														grow_factor = 0,
														vertical_grow = false,
														horizontal_alignment = "left",
														T.label {
															id = "title_2",
															label = "<b>Active mods</b>\n ",
															use_markup=true,
														},
													},
												},
												T.row {
													grow_factor = 1,
													T.column {
														grow_factor = 0,
														horizontal_grow=true,
														vertical_alignment="top",
														T.scroll_label {
															id = "label_activemods",
														},
													},
												},
											}
										}
									}
								},
							},
						}
					}
				}
			}
		},
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
	}
}
