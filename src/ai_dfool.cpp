/* $Id$ */
/*
   Copyright (C) 2007 - 2008
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version
   or at your option any later version2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file ai_dfool.cpp
//!

#include "global.hpp"

#include "ai_dfool.hpp"
#include "log.hpp"
#include "variable.hpp"

#include <set>

namespace dfool {
  void dfool_ai::play_turn(){
    info info_ = get_info();
    int team_num=get_info().team_num;
    const config& parms = current_team().ai_parameters();
    config ai_mem = current_team().ai_memory();

    const config::child_list& orders = parms.get_children("order");
    LOG_STREAM(info, ai)<<"dfool side:"<<team_num<<" of "<<current_team().nteams()<<std::endl;

    config side_filter;
    char buf[80];
    snprintf(buf, sizeof(buf), "%d", team_num);
    side_filter["side"]=buf;

    LOG_STREAM(info, ai)<<"dfool sees:"<<std::endl;

    //    for(unit_map::iterator ua = get_info().units.begin(); ua != get_info().units.end(); ++ua) {
    //        std::string t = ua->second.get_ai_special();
    //        LOG_STREAM(info, ai)<<"ua:"<<ua->second.underlying_id()<<"\t"<<t<<std::endl;
    //        LOG_STREAM(info, ai)<<"\t\t\t"<<ua->first.x<<","<<ua->first.y<<std::endl;
    //    }

    unit_list all = all_units();
    unit_list my_units=filter_units(side_filter, all,get_info().units);
    unit_list v_units=visible_units();

    //    LOG_STREAM(info, ai)<<"My Units"<<std::endl;
    //    for(unit_list::iterator ui = all.begin(); ui != all.end(); ++ui) {
    //      LOG_STREAM(info, ai)<<"\t....."<<*ui<<"........"<<std::endl;
    //      unit_map::iterator u = unit(*ui,get_info().units);
    //      if(u!=get_info().units.end()){
    //	LOG_STREAM(info, ai)<<"\t"<<u->second.name()<<std::endl;
    //	LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_id()<<std::endl;
    //	LOG_STREAM(info, ai)<<"\t\t\t"<<u->second.get_ai_special()<<std::endl;
    //	LOG_STREAM(info, ai)<<"\t\t\t"<<u->first.x<<","<<u->first.y<<std::endl;
    //      }
    //    }

    LOG_STREAM(info, ai)<<"Visible Units"<<std::endl;
    for(unit_list::iterator ui = v_units.begin(); ui != v_units.end(); ++ui) {
      unit_map::iterator u = unit(*ui,get_info().units);
      if(u!=get_info().units.end()){
	//	LOG_STREAM(info, ai)<<"\t"<<u->second.name()<<std::endl;
	LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_id()<<std::endl;
	//	LOG_STREAM(info, ai)<<"\t\t\t"<<u->second.get_ai_special()<<std::endl;
	//	LOG_STREAM(info, ai)<<"\t\t\t"<<u->first.x<<","<<u->first.y<<std::endl;

	unit_memory_.add_unit_sighting(u->second, u->first, get_info().state.turn());
      }
    }

    for(config::child_list::const_iterator o = orders.begin(); o != orders.end(); ++o) {
      std::string order_id=(**o)["id"];
      std::string number=(**o)["number"];
      size_t num=atoi(number.c_str());
      const config::child_list& filter = (**o).get_children("filter");

      LOG_STREAM(info, ai)<<"dfool order:"<<order_id<<std::endl;

      // First find units where AI_SPECIAL matches order id
      config order_filter;
      order_filter["ai_special"]=order_id;
      unit_list order_units = filter_units(order_filter,my_units,get_info().units);
      if(num > order_units.size()){
	// Need to populate orders
	// Find units that match any filter.
	// If no filters, then accept all units.
	if(filter.size()){
	  for(config::child_list::const_iterator f = filter.begin(); f != filter.end(); ++f) {
	    config ff=**f;
	    //            LOG_STREAM(info, ai)<<"dfool filter:"<<std::endl;
            unit_list filtered_units=filter_units(ff,my_units,get_info().units);

	    //! @todo FIXME: add sorting

            for(unit_list::iterator i = filtered_units.begin(); i != filtered_units.end() && (num > order_units.size()); ++i) {
	      unit_map::iterator ui=unit(*i,get_info().units);
	      if(ui!=get_info().units.end()){
		std::string ais=ui->second.get_ai_special();

		//		LOG_STREAM(info, ai)<<"\t match: "<<ui->second.underlying_id()<<"\t"<<ais<<":::"<<std::endl;

		bool used=(ais.size() > 0);
		if(used){
		  //		  LOG_STREAM(info, ai)<<"\t\talready assigned: "<<ui->second.underlying_id()<<"\t"<<ais<<std::endl;
		}else{
		  ui->second.assign_ai_special(order_id);
		  order_units.push_back(*i);

		  //		  LOG_STREAM(info, ai)<<"\t\tmatching: "<<ui->second.underlying_id()<<" to order: "<<order_id<<"\t"<<ui->second.get_ai_special()<<std::endl;
		}
	      }
            }
          }
        }else{
          order_units=my_units;
        }
      }

      // Execute commands
      for(unit_list::iterator ou = order_units.begin(); ou != order_units.end(); ou++){
        const config::child_list& commands = (**o).get_children("command");
	bool com_break=false;

        for(config::child_list::const_iterator com = commands.begin(); com != commands.end() && !com_break; ++com) {
	  unit_map::iterator u=unit(*ou,get_info().units);

	  const config::child_list& com_filter = (**com).get_children("filter");
	  bool found=true;
	  if(u!=get_info().units.end()){
	    for(config::child_list::const_iterator sf = com_filter.begin(); sf != com_filter.end(); ++sf) {
	      config ff=**sf;
	      LOG_STREAM(info, ai)<<"ff:"<<(**com)["type"]<<" "<<ff["type"]<<" "<<ff["x"]<<","<<ff["y"]<<std::endl;
	      LOG_STREAM(info, ai)<<"ff?"<<u->second.type_id()<<" "<<u->first.x<<","<<u->first.y<<std::endl;
	      if(! u->second.matches_filter(&ff,u->first)) {
		found=false;
		break;
	      }
	    }
	  }

	  if(found){
	    std::string type=(**com)["type"];
	    std::string e=(**com)["test"];
	    std::map<std::string, evaluator*> function_map;
	    arithmetic_evaluator eval(get_info().state.sog(),&function_map);
	    distance_evaluator dist(get_info().state.sog(),&function_map);
	    function_map["eval"]=&eval;
	    function_map["distance"]=&dist;
	    std::cout<<"eval: "<<type<<":"<<e<<" = "<<eval.value(e)<<"\n";

	    LOG_STREAM(info, ai)<<"\tcommand: "<<type<<std::endl;
	    if(type=="moveto"){
	      moveto(com,u);
	    }
	    if(type=="set_order"){
	      std::string set_id=(**com)["id"];
	      std::string a=(u->second.get_ai_special());
	      LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_id()<<"\t"<<a<<"->"<<set_id<<std::endl;
	      (u->second.assign_ai_special(set_id));
	      a=(u->second.get_ai_special());
	      LOG_STREAM(info, ai)<<"\t\t"<<u->second.underlying_id()<<"\t"<<a<<" =?= "<<set_id<<std::endl;
	    }
	    if(type=="break"){
	      com_break=true;
	    }
          }
        }
      }
    }

    unit_memory_.write(ai_mem);
    current_team().set_ai_memory(ai_mem);

    return;
  }

  unit_list dfool_ai::filter_units(const config& filter, unit_list& ul, unit_map& um)
  {
    //    LOG_STREAM(info, ai)<<"filter1:"<<std::endl;
    unit_list filtered_units_;
    for(unit_list::const_iterator i = ul.begin(); i != ul.end(); ++i) {
      //      LOG_STREAM(info, ai)<<"filter2:"<<std::endl;
      unit_map::iterator j = unit(*i,um);
      //      LOG_STREAM(info, ai)<<"j:"<<j->second.underlying_id()<<":"<<j->first.x<<","<<j->first.y<<std::endl;
      if(j->second.underlying_id().size()>0){
	//	LOG_STREAM(info, ai)<<"filter3:"<<std::endl;
	if(j->second.matches_filter(&filter,j->first)) {
	  //	  LOG_STREAM(info, ai)<<"filter4:"<<std::endl;
	  filtered_units_.push_back(*i);
	}
      }
    }
    //    LOG_STREAM(info, ai)<<"filter:"<<std::endl;
    return filtered_units_;
  }

  unit_list dfool_ai::visible_units()
  {
    unit_list visible_units;
    bool no_fog=current_team().uses_shroud() == false && current_team().uses_fog() == false;

    const unit_map& um=get_info().units;
    for(unit_map::const_iterator i = um.begin(); i != um.end(); ++i) {
      bool hidden_by_fog = current_team().fogged(i->first);
      bool hidden = i->second.invisible(i->first, um, get_info().teams);
      if((no_fog || !hidden_by_fog) && !hidden) {
	  visible_units.push_back(i->second.underlying_id());
      }
    }

    LOG_STREAM(info, ai) << "number of visible units: " << visible_units.size() << "\n";
    return visible_units;
  }

  unit_list dfool_ai::all_units(){
    unit_list all;
    const unit_map& um=get_info().units;
    for(unit_map::const_iterator i = um.begin(); i != um.end(); ++i) {
      //      LOG_STREAM(info, ai)<<"all:"<<i->second.underlying_id()<<std::endl;
      all.push_back(i->second.underlying_id());
    }
    return(all);
  }

  bool dfool_ai::moveto(config::child_list::const_iterator o, unit_map::const_iterator m){
      location target(atoi((**o)["target_x"].c_str())-1,atoi((**o)["target_y"].c_str())-1);
      LOG_STREAM(info, ai)<<"\tmoving to:("<<target.x<<","<<target.y<<")"<<std::endl;
      if(m->second.movement_left()){
	std::map<location,paths> possible_moves;
	move_map srcdst, dstsrc;
	unit_map known_units;

	//	unit_memory_.known_map(known_units, get_info().state.turn());
	unit_memory_.known_map(known_units, 0);

	std::cout<<"known units:\n";
	for(unit_map::const_iterator uu = known_units.begin();uu!=known_units.end();uu++){
	  std::cout<<"\t"<<uu->second.underlying_id()<<" "<<uu->first<<"\n";
	}

	calculate_moves(known_units,possible_moves,srcdst,dstsrc,false,false,NULL,true);

	int closest_distance = -1;
	std::pair<location,location> closest_move;

	//! @todo This undoubtedly could be done more cleanly
	for(move_map::const_iterator i = dstsrc.begin(); i != dstsrc.end(); ++i) {
	  // Must restrict move_map to only unit that is moving.
	  if(i->second==m->first){
	    const int distance = distance_between(target,i->first);
	    //	    int distance=10000;
	    //	    std::cout<<"got here\n";
	    //	    const shortest_path_calculator calc(m->second, current_team(),known_units,get_info().teams,get_info().map);
	    //std::cout<<"got here2\n";
	    //paths::route route = a_star_search(m->first, target, 1000.0, &calc,
	    //get_info().map.x(), get_info().map.y());
	    //	    std::cout<<"got here3\n";

	    //	    distance = route_turns_to_complete(m->second, get_info().map, route, known_units, get_info().teams) + distance_between(target,i->first)/100;

	    if(closest_distance == -1 || distance < closest_distance) {
			  closest_distance = distance;
			  closest_move = *i;
	    }
	  }
	}

	LOG_STREAM(info, ai)<<"\tmoving : "<< m->second.underlying_id() <<" "<<" from ("<<closest_move.first.x<<","<<closest_move.first.y<<")"<<" to ("<<target.x<<","<<target.y<<")"<<std::endl;
	LOG_STREAM(info, ai)<<"\tdistance: "<<closest_distance<<"\n";

	if(closest_distance != -1) {
	  gamemap::location to = move_unit_partial(closest_move.second,closest_move.first,possible_moves);
	  if(to != closest_move.second)
	    return(false);	// Something unexpected happened
	}
      }
      return(true);
  }

  unit_map::iterator dfool_ai::unit(std::string unit_id, unit_map& um){
    //    LOG_STREAM(info, ai)<<"unit start:"<<unit_id<<std::endl;
    for(unit_map::iterator u = um.begin(); u != um.end(); u++){
      if(unit_id == u->second.underlying_id()){
	//	LOG_STREAM(info, ai)<<"unit:"<<unit_id<<" , "<< u->second.underlying_id()<<std::endl;
	return u;
      }
    }
    //    LOG_STREAM(info, ai)<<"nounit:"<<std::endl;
    return(um.end());
  }

  unit_memory::unit_memory(const game_data& gamedata, const config& cfg) : 
  	units_(),
	ids_(),
	turns_(),
	locations_()
  {
    const config::child_list mem_list=cfg.get_children("unit_memory");
    for(config::child_list::const_iterator mem = mem_list.begin(); mem != mem_list.end(); ++mem) {
      config unit_cfg = *((*mem)->child("unit"));

      unit u(gamedata , unit_cfg);

      int t = atoi((**mem)["turn"].c_str());

      gamemap::location l(atoi((**mem)["x"].c_str())-1,atoi((**mem)["y"].c_str())-1);
      add_unit_sighting(u,l,t);
    }
  }

  void unit_memory::add_unit_sighting(unit u, gamemap::location l, size_t t){
    std::string unit_id= u.underlying_id();
    // Check if this unit has already been seen
    size_t i,j;
    for(i=0; i < ids_.size();i++){
      if(unit_id == ids_[i]){break;}
    }

    if(i == ids_.size()){
      // Unit has not been seen
      units_.push_back(u);
      ids_.push_back(unit_id);
      turns_.push_back(t);
      locations_.push_back(l);
    }else{
      // Update unit info
      units_[i]=u;
      turns_[i]=t;
      locations_[i]=l;
    }

    // Remove units that are co-located units
    std::set<size_t> remove_list;
    for(j=0; j < ids_.size();j++){
      if(j!=i && locations_[j] == locations_[i]){
	  remove_list.insert(j);
      }
    }
    for(std::set<size_t>::const_iterator k=remove_list.begin();k!=remove_list.end();k++){
      remove_unit_sighting(ids_[*k]);
    }
  }

  void unit_memory::remove_unit_sighting(std::string id){
    size_t i;
    for(i=0;i<ids_.size();i++){
      if(id == ids_[i]){break;}
    }

    if(i == ids_.size()){
      // Unit not in memory
    }else{
      // Remove unit info
      units_.erase(units_.begin()+i);
      ids_.erase(ids_.begin()+i);
      locations_.erase(locations_.begin()+i);
      turns_.erase(turns_.begin()+i);
    }
  }

  void unit_memory::write(config& temp){
    //    std::cout<<"ai_write:\n";
    for(size_t i = 0; i < units_.size(); i++){
      config element;
      write_element(i, element);
      temp.add_child("unit_memory",element);
    }
  }

  void unit_memory::write_element(int i, config &temp){
    config temp_unit;
    std::stringstream ts,xs,ys;
    ts << turns_[i];;
    temp["turn"] = ts.str();
    xs << locations_[i].x;
    temp["x"] = xs.str();
    ys << locations_[i].y;
    temp["y"] = ys.str();
    units_[i].write(temp_unit);
    temp.add_child("unit",temp_unit);
    //    std::cout<<"ai write: "<<temp_unit["id"]<<"\n";
  }

  void unit_memory::known_map(unit_map& u, size_t turn){
    size_t i;
    std::map<gamemap::location,size_t> turn_used;
    for(i=0;i<units_.size();i++){
      gamemap::location l = locations_[i];
      size_t t = turn_used[l];
      if(turns_[i] >= turn && turns_[i] >= t){
	   //      std::cout<<"turn_used: "<< t <<"\n";
	 //      std::cout<<"turn: "<< turns_[i] <<"\n";
	  turn_used[l] = t;
	  if(t != 0){
	    u.replace(new std::pair<gamemap::location,unit>(l,units_[i]));
	  }else{
	    std::cout<<"id: "<< ids_[i] <<"\n";

	    u.add(new std::pair<gamemap::location,unit>(l,units_[i]));
	  }
      }
    }
  }

  std::string evaluator::value(const std::string& val_string){
    std::string temp_string = val_string;

    std::vector<std::string> p = utils::paranthetical_split(val_string,0,"(",")");

    // Find function calls designated by @ and evaluate values inside ()
    std::string func;
    std::stringstream tot;
    std::cout<<"got here eval:"<<val_string<<"\n";
    bool function=false;
    for(size_t i=0;i!=p.size();i++){
	 std::stringstream ptemp;
	 if(i%2){
	   if(function){
		std::cout<<"function: "<<func<<"\n";
		std::map<std::string, evaluator*>::iterator fmi =
		  function_map_->find(func);
		if(fmi != function_map_->end()){ // evaluate function
		  std::cout<<"function ::: "<<func<<" ::: "<<fmi->first<<"\n";
		  evaluator& f=*(fmi->second);
		  ptemp<<f.value(p[i]);
		  p[i]=ptemp.str();
		}else{ // error
		  std::cout<<"function undefined: "<<func<<"\n";
		  LOG_STREAM(info, ai)<<"error: evaluator function undefined: "<<func<<"\n";
		  p[i]="ERR";
		}
	   }else if(p[i].size()>0 ){
		p[i]="("+p[i]+")";
	   }
	   function=false;
	 }else{
	   std::string t=p[i];
	   std::cout<<"got here: t :"<<t<<"\n";
	   std::vector<std::string> temp=utils::split(t,'@');
	   std::cout<<"got here: temp :"<<temp.size()<<"\n";

	   if(std::find(t.begin(),t.end(),'@') != t.end()){
		function=true;
	   }

	   if(temp.size()>2){
		LOG_STREAM(info, ai)<<"evaluator syntax error:\n\t" << val_string << std::endl;
		std::cout<<"evaluator syntax error:\n\t" << val_string << std::endl;
	   }
	   std::cout<<"eval size:"<<temp.size()<<"\n";

	   if(function && temp.size()>0){
		std::cout<<"temp "<<temp[0]<<"\n";
		if(temp.size()==2){
		  p[i]=temp[0];
		  func=temp[1];
		}else{
		  std::cout<<"got lost::"<<temp[0]<<"\n";
		  p[i]="";
		  func=temp[0];
		}
	   }else{
		p[i]=temp[0];
	   }
	 }
	 tot<<p[i];
    }
    return(utils::interpolate_variables_into_string(tot.str(),state));
  }

  std::string arithmetic_evaluator::value(const std::string& val_string){
    std::string temp = evaluator::value(val_string); // calculate WML variables
    std::list<std::string> tokens = parse_tokens(temp);
    std::cout<<"tokens:\n";
    for(std::list<std::string>::const_iterator i=tokens.begin();i!=tokens.end();i++){
	 std::cout<<"\t"<<(*i)<<"\n";
    }
    if(tokens.size()){
	 std::cout<<"got here tokenless\n";
	 temp=evaluate_tokens(tokens);
    }
    std::cout<<"temp:"<<temp<<"\n";
    return temp;
  }

  std::string arithmetic_evaluator::evaluate_tokens(std::list<std::string> &tlist){
    std::vector<std::string> op_priority;
    op_priority.push_back("^");
    op_priority.push_back("*/%");
    op_priority.push_back("+-");
    double temp=0;
    std::cout<<"got here token\n";
    for(size_t i=0;i<op_priority.size();i++){
	 tlist.remove("");
	 for(std::list<std::string>::iterator token = tlist.begin();token!=tlist.end();token++){
	   for(size_t j=0;j<op_priority[i].size();j++){
		std::string t;
		t+=op_priority[i][j];
		if((*token) == t){
		  std::list<std::string>::iterator a=token;
		  std::list<std::string>::iterator b=token;
		  std::cout<<"got here token1\n";
		  a--;
		  b++;
		  std::cout<<"atb:"<<*token<<"\n";
		  //		  std::cout<<"atb:"<<*a<<*token<<*b<<"\n";
		  if((*token)[0]=='*'){
		    temp= atof((*a).c_str()) * atof((*b).c_str());
		  }
		  if((*token)[0]=='/'){
		    temp= atof((*a).c_str()) / atof((*b).c_str());
		  }
		  if((*token)[0]=='%'){
		    temp= std::fmod(atof((*a).c_str()), atof((*b).c_str()));
		  }
		  if((*token)[0]=='+'){
		    temp= atof((*a).c_str()) + atof((*b).c_str());
		  }
		  if((*token)[0]=='-'){
		    temp= atof((*a).c_str()) - atof((*b).c_str());
		  }
		  if((*token)[0]=='^'){
		    temp= std::pow(atof((*a).c_str()),atof((*b).c_str()));
		  }
		  std::cout<<"got here token2:"<<temp<<"\n";
		  std::stringstream r;
		  r<<temp;
		  *a="";
		  *token="";
		  *b=r.str();
		  token++;
		  //		  tlist.erase(a);
		  //		  tlist.erase(b);
		  break;
		}
	   }
	 }
    }
    tlist.remove("");
    return(*(tlist.begin()));
  }

  std::list<std::string> arithmetic_evaluator::parse_tokens(const std::string& s){
    std::list<std::string> ret;
    std::string temp;
    std::string str="";
    std::string operators = "^*/%+-";
    std::string digits = "0123456789";
    char dpoint='.';
    std::string parenthesis="()";
    int count=0;
    size_t i;

    for(i=0;i!=s.size();i++){ // strip out spaces
	 if(s[i]!=' '){
	   str+=s[i];
	 }
    }

    i=0;
    while(i!=str.size()){
	 std::cout<<"i:"<<i<<"\n";
	 if(0==count){
	   ret.push_back("");
	 }
	 char c=str[i];
	 bool dpfound=false;
	 bool found=false;
	 for(size_t j=0;j!=digits.size();j++){
	   if(c==digits[j]){
		found=true;
		break;
	   }
	 }

	 if(!found && !dpfound && c==dpoint){ // check for decimal point
	   dpfound=true;
	   found=true;
	 }

	 if(found || (0==count && c=='-')){
	   ret.back()+=c;
	   count++;
	 }else if(count){
	   count=0;
	   for(size_t j=0;j!=operators.size();j++){
		if(c==operators[j]){
		  found=true;
		  break;
		}
	   }
	   if(found){
		ret.push_back("");
		ret.back()+=c;
	   }else{
		std::cout<<"error in arithmetic operator:"<<c<<":\n\t"<<s<<"\n";
		return(ret);
	   }
	 }else if(c==parenthesis[0]){
	   std::vector<std::string> temp=utils::paranthetical_split(str.substr(i,str.size()-i),0);
	   if(temp.size()%2!=0){
		std::cout<<"temp"<<temp[1]<<":"<<i<<"\n";
		ret.back()+=value(temp[1]);
		i+=temp[1].size()+2;
		std::cout<<"\t"<<ret.back()<<"\n";
		std::cout<<"temp 2:"<<i<<"\n";
		count=0;
		c=str[i];
		for(size_t j=0;j!=operators.size();j++){
		  if(c==operators[j]){
		    found=true;
		    break;
		  }
		}
		if(found){
		  ret.push_back("");
		  ret.back()+=c;
		}else if(i!=str.size()){
		  std::cout<<"error in arithmetic operator:"<<str[i]<<":\n\t"<<s<<"\n\t"<<str.substr(0,i)<<"\n\t"<<str.substr(i,str.size()-i)<<"\n";
		  return(ret);
		}
	   }else{
		std::cout<<"error in parenthetical expression:\n\t"<<s<<"\n";
	   }
	 }else if(i!=str.size()){
	   std::cout<<"error in arithmetic expression:\n\t"<<s<<":\n\t"<<str.substr(0,i)<<"\n\t:"<<s[i]<<":\n";
	   std::cout<<i<<";"<<str.size()<<"\n";
	   return(ret);
	 }
	 if(i!=str.size()){
	   i++;
	 }
    }
    std::cout<<"got here 5\n";
    return(ret);
  }

  std::string distance_evaluator::value(const std::string& val_string){
    std::cout<<"got distance:"<<val_string<<"\n";
    std::vector<std::string> vals = utils::split(val_string,',');
    if(vals.size()<4){
	 std::cout<<"error in distance parameters\n";
	 return("ERR");
    }
    std::vector<int> coord;
    for(size_t i=0;i!=4;i++){
	 vals[i] = arithmetic_evaluator::value(vals[i]);
	 std::cout<<"vals["<<i<<"]:"<<vals[i]<<"\n";
	 coord.push_back(atoi(vals[i].c_str()));
    }
    std::cout<<"coords:"<<coord[0]<<","<<coord[1]<<"\n";
    std::cout<<"coords:"<<coord[2]<<","<<coord[3]<<"\n";
    gamemap::location a(coord[0]-1,coord[1]-1);
    gamemap::location b(coord[2]-1,coord[3]-1);
    std::cout<<"a "<<a.x<<","<<a.y<<"\n";
    std::cout<<"b "<<b.x<<","<<b.y<<"\n";
    int distance = distance_between(a,b);
    std::stringstream t;
    t<<distance;
    return(t.str());
  }
} // end namespace dfool
