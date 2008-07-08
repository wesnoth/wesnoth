# This file is auto-generated from the current state of the database. Instead of editing this file, 
# please use the migrations feature of Active Record to incrementally modify your database, and
# then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your database schema. If you need
# to create the application database on another system, you should be using db:schema:load, not running
# all the migrations from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended to check this file into your version control system.

ActiveRecord::Schema.define(:version => 19) do

  create_table "campaign_names", :force => true do |t|
    t.string "name"
  end

  create_table "difficulty_names", :force => true do |t|
    t.string "name"
  end

  create_table "games", :force => true do |t|
    t.integer "player_id",          :limit => 11
    t.integer "version_name_id",    :limit => 11
    t.integer "campaign_name_id",   :limit => 11
    t.integer "scenario_name_id",   :limit => 11
    t.integer "difficulty_name_id", :limit => 11
    t.integer "number_turns",       :limit => 11
    t.integer "start_turn",         :limit => 11
    t.integer "end_turn",           :limit => 11
    t.integer "start_time",         :limit => 11
    t.integer "end_time",           :limit => 11
    t.integer "gold",               :limit => 11
    t.integer "end_gold",           :limit => 11
    t.integer "status",             :limit => 11
    t.string  "serial",             :limit => 30
  end

  create_table "players", :force => true do |t|
    t.string "unique_player_id", :limit => 30
  end

  create_table "scenario_names", :force => true do |t|
    t.string "name"
  end

  create_table "special_unit_names", :force => true do |t|
    t.string "name"
  end

  create_table "special_units", :force => true do |t|
    t.integer "game_id",              :limit => 11
    t.integer "level",                :limit => 11
    t.integer "special_unit_name_id", :limit => 11
    t.integer "experience",           :limit => 11
  end

  create_table "unit_names", :force => true do |t|
    t.string "name"
  end

  create_table "units", :force => true do |t|
    t.integer "game_id",      :limit => 11
    t.integer "level",        :limit => 11
    t.integer "unit_name_id", :limit => 11
    t.integer "count",        :limit => 11
  end

  create_table "version_names", :force => true do |t|
    t.string "name"
  end

end
