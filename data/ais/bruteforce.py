#!WPY

import wesnoth, random

##    Copyright 2006 by Michael Schmahl
##    This code is available under the latest version of the GNU Public License.
##    See COPYING for details.  Some inspiration and code derived from "sample.py"
##    by allefant.
##
##    This is my attempt at a 'chess-like' AI.  All moves are motivated by
##    an underlying evaluation function.  The actual eval function doesn't
##    need to be coded, because moves can be scored and ranked based on the
##    incremental change in the evaluation.  Unlike a chess-playing program,
##    though, this program does no lookahead, because the branching factor
##    is prohibitively high (potentially in the thousands), and because then
##    the script would have to create an internal model of the game state.
##
##    Despite the lack of any lookahead, I still consider this AI to be
##    chess-like because it evaluates every possible move and attack, even
##    those that are obviously (to a human) bad.  How can a computer know
##    that these are bad moves unless it actually checks?
##
##    The evaluation function is:
##
##    (1)     side_score = village_score
##                         + sum(unit_score, over all units)
##                         + positional_score
##
##    The value of a unit can be highly subjective, but to simplify, assume
##    that any level-1 unit is just as valuable as any other level-1 unit.
##    Specifically, the value of a unit will be:
##
##    (2)     unit_score = (1 + level + %xp)(1 + %hp)
##
##    Leaders are be considered three levels higher than their actual level.
##    So a freshly-recruited level-1 unit is worth 4.0 points.  And a level-2
##    unit with half its hitpoints remaining, but halfway to level 3, is
##    worth 6.75 points.
##
##    One question is: How much is a village worth, compared to a (typical)
##    unit?  A typical unit is worth 15 to 20 gold, because that is how much
##    we paid for it.  A village is worth two or three gold *per turn* as
##    long as it is held.  (The village is worth three gold when it offsets
##    a unit's upkeep.)  So we must make some assumptions as to the value of
##    a present gold piece, compared to a future gold piece.  Assume a decay
##    rate of 1.5 (i.e. a gold piece one turn from now is worth two-thirds
##    of a gold piece now).  This makes the present value of a village equal
##    to twice its income.  If we set the value of a typical unit at 16 gold,
##    we get that an upkeep-offsetting village is worth 1.5 points, and a
##    supernumerary village is worth 1.0 points.  For simplicity, the value
##    of each village is set at 1.0.
##
##    (3)     village_score = number of villages
##
##    The positional score is the most interesting term of equation (1),
##    because that, more than anything else, will guide the AI's behavior.
##
##    First, we want the AI to expand to capture villages.  So, for each unit,
##    it is scored based on how far it is from the nearest unowned or enemy
##    village.  If the distance is zero, the unit has actually captured the
##    village, so in that limit, the value should be equal to the village
##    value.  As the distance approaces infinity, the score should tend
##    toward zero.  This suggests something like:
##
##    (4)     village_proximity = c / (c + distance)
##
##    I have selected c to be equal to equal to the unit's movement.   This
##    means that (approximately) a unit one turn away from capturing a village
##    gets 0.5 points; two turns, 0.33 points, etc.  Although an exponential
##    relationship would be more accurate, exponentiation is expensive, and
##    better avoided, since thousands of moves are evaluated per turn.
##
##    Second, we want units to stand on defensive terrain when within range
##    of the enemy.  The 'right' way to do this would be to count up all the
##    potential attackers at the destination square, see how much damage they
##    might do, and score the move based on how much damage would be dealt/
##    prevented.  Again, this is much too slow.  I have found a reasonable
##    approximation is:
##
##    (5)     exposure_penalty = -defense_modifier / 10
##
##    Maybe much too simple, but easy to calculate!  In future editions, perhaps
##    I should take into account how damaged the unit is, or at least make some
##    attempt to count the attackers.
##
##    Third, we want units to heal when damaged or poisoned.  Referring to
##    equation (2), we can see that the value of healing is:
##
##    (6)     healing_score = healing / max_hitpoints * (1 + level + %xp)
##
##    We consider poison, which does 8 damage *per turn*, to be equivalent to
##    16 points of actual damage, for the same reason a village's real value is
##    twice its income (see above).
##
##    Fourth, we want units to guard villages if the enemy is in range to take
##    them.  If, by stationing a unit on a village, we prevent the enemy from
##    taking it, we have prevented a 2-point swing in the enemy's favor.  Again
##    considering a decay rate of 2/3 per turn, this means the garrison value
##    is 4/3.  But since there is no guarantee that our garrison will be
##    successful (perhaps the enemy will take the village anyway; perhaps it is
##    not possible to garrison all threatened villages), we will cut this in half.
##
##    (7)     garrison_score = 2/3
##
##    Fifth, we want our leader to stay near a keep.  Otherwise, any gold we
##    might have accumulated will be wasted.  And finally, we want units to move
##    toward the enemy leader.  These are accomplished by treating keeps as
##    if they were unowned villages (for our leader), and the enemy leader
##    as if it were a village (for everyone else).
##
##    This should be all that is required to play a decent game of Wesnoth.
##    This AI scores quite well against the Wesnoth default AI, which may be
##    surprising, because it uses no sophisticated tools.  There is no attempt
##    to use any of the path-finding tools made available by the API (which
##    would be too slow to be used thousands of times every turn).  There is
##    no attempt to use combination attacks (meaning, that even though none of
##    several units can favorably attack a certain target, if they all attack
##    in the same turn, the result is likely to be favorable).  No attempt is
##    made to assign units individually to targets.
##
##    Some bad behaviors may result from these shortcomings:
##
##    If the map is maze-like, or simply has a few corners surrounded by
##    impassable terrain, units may get stuck.  On Cynsaun Battlefield, for
##    example, a group of units got stuck in the middle of the river, trying
##    to capture a village on the other side of the deep-water hexes.
##
##    An enemy unit may get completely surrounded by friendly units, who are
##    weak in comparison to the enemy, and our AI will make no attempt to kill
##    the enemy unit.  (Think six Wolf Riders surrounding an Orcish Grunt.)
##    Usually one or more of these units will find something else to do, allowing
##    a few Archers to take their place and start to wear down the Grunt.  Or
##    the Grunt will attack, getting damaged in the process, and creating a
##    chance-to-kill for one of the Wolves.
##
##    If there is an unoccupied village in a corner of the map, our AI will
##    send every unit that is closer to the village than any other, to that
##    village.  Often, only one unit is necessary.  Thus, harassing villages
##    with scouts may be a much more viable strategy against this AI than
##    against human players, or against the default AI.
##
##    For those interested in results, I have set up a tournament between my
##    AI and the default AI.  The tournament consists of one match on each of
##    the mainline two-player maps (except Wesbowl, naturally).  In each map,
##    each opponent is allowed to be player 1 once.  If there is no decision
##    after two games, two more games are played, repeating as necessary until
##    one opponent has won the match.  All games are played with a 50-turn
##    limit, 2 gold per village, 70% experience, and no fog.  (I think there
##    is a bug (feature?) that AIs ignore fog, so I disabled it to improve the
##    observer's (my) experience.)  Factions are chosen randomly.
##
##    Map                           W-L-D   %Win   Match result
##    Blitz                         2-0-0    100   Win
##    Caves of the Basilisk         4-2-0     67   Win
##    Charge                        3-1-0     75   Win
##    Cynsaun Battlefield (1gpv)    2-0-0    100   Win
##    Den of Onis                   4-2-0     67   Win
##    Hamlets                       2-0-0    100   Win
##    Hornshark Island              0-2-0      0   Loss
##    Meteor Lake                   2-0-0    100   Win
##    Sablestone Delta              2-0-0    100   Win
##    Silverhead Crossing           3-1-0     75   Win
##    Sulla's Ruins                 2-0-0    100   Win
##    ** Overall                   25-8-0     76   10 Wins, 1 Loss (91%)

# UNIT SCORE MODIFIERS

BASE_UNIT_SCORE = 1 # Base worth of a unit
LEVEL_SCORE = 1     # Worth/level
LEADER_SCORE = 3    # Leader worth
FULL_XP_SCORE = 1   # How much is partial XP worth (1 is 100% XP = 1 pt)

# This score is then multiplied by a factor dependant on the price of the unit
# this makes expensive units worth more to the AI

COST_SCORE = 0      #
BASE_COST_SCORE = 1 #

# Formula:
# Base_Score = BASE_UNIT_SCORE + level * LEVEL_SCORE + is_leader * LEADER_SCORE + xp/max_xp * FULL_XP_SCORE
# Cost_Modifier = BASE_COST_SCORE + price * COST_SCORE
# Unit_Score(unit_k) = Base_Score * Cost_Modifier

# POSITION SCORE MODIFIERS

NO_MOVE_PEN = 0             # Penalty for not moving (doesn't quite work)
NEXT_TO_ENEMY_PEN = 0       # Penalty for moving next to an enemy and not attacking
STAND_NEXT_TO_ENEMY_PEN = 0 # Penalty for standing next to an enemy without moving or attacking

# MISC SCORE MODIFIERS

LEVEL_CHANCE_BONUS = 0      # How much a level-up is worth

VILLAGE_SCORE = 1           # How much capturing a village is worth
ENEMY_VILLAGE_BONUS = 1     # How much extra is an enemy village worth

GARRISON_SCORE = 2.0/3      # How much defending a village is worth
DEFENSE_FACTOR = 1.0/1000   # How much to penalize a unit for being in an attackable position

HEAL_FACTOR = 1             # How much is healing worth
HEAL_ATTACKABLE = .5        # How much relative to healing is healing when attackable worth
HEAL_POISON = 16            # How much is healing from poison worth

HP_SCALE = .1               # Unit HP/turn (for recruitment)

def pos(p):
    if p==None: return ("Nowhere")
    return ("(%s,%s)"%(p.x+1,p.y+1))

class AI:
    def __init__(self):
        self.get_villages()
        self.get_keeps()
        self.mapsize = max((wesnoth.get_map().x,wesnoth.get_map().y)) / 30.0
        self.stats = [0,0]

    def report_stats(self):
        wesnoth.log_message("%d moves, %d fights evaluated" % (self.stats[0],self.stats[1]))

    def get_villages(self):
        self.notmyvillages = []
        m = wesnoth.get_map()
        for x in range(m.x):
            for y in range(m.y):
                loc = wesnoth.get_location(x,y)
                if m.is_village(loc):
                    for team in wesnoth.get_teams():
                        if team.owns_village(loc) and not team.is_enemy:
                            break
                    else:
                        self.notmyvillages.append(loc)

    def get_keeps(self):
        self.keeps = []
        m = wesnoth.get_map()
        for x in range(m.x):
            for y in range(m.y):
                loc = wesnoth.get_location(x,y)
                if m.is_keep(loc):
                    # If the enemy is occupying the keep, it is "off-limits" to our leader.
                    # Otherwise, if our leader has strayed too far, it might attempt to go
                    # to the enemy keep, which basically means we lose.
                    if loc not in wesnoth.get_enemy_destinations_by_unit().keys():
	                    self.keeps.append(loc)
        
    def recruit(self):
        # I haven't discussed this at all.  Perhaps a few paragraphs would be in order.
        if wesnoth.get_current_team().gold < 16: return
        
        # find our leader
        leaderpos = None
        for location,unit in wesnoth.get_units().iteritems():
            if unit.can_recruit and unit.side == wesnoth.get_current_team().side:
                leaderpos = location
                break

        # no leader? can't recruit
        if leaderpos == None: return

        # is our leader on a keep?  If not, move to a keep
        # Maybe should always go to nearest keep
        if not leaderpos in self.keeps:
            for dest in wesnoth.get_destinations_by_unit().get(leaderpos,[]):
                if dest in self.keeps:
                    leaderpos = wesnoth.move_unit(leaderpos,dest)
                    break

        # is our leader on a keep now?  If not, can't recruit
        if leaderpos not in self.keeps: return

        # build up a list of recruits and scores for each
        recruit_list = []
        sumweights = 0
        for recruit in wesnoth.get_current_team().recruits():
            weight = self.recruit_score(recruit)
            if weight < 0.01: weight = 0.01
            recruit_list.append((recruit.name,weight))
            sumweights += weight

        # repeatedly recruit until we fail
        while 1:

            # pick a random recruit in proportion to the weights
            r = random.uniform(0,sumweights)
            for recruit,weight in recruit_list:
                r -= weight
                if r < 0: break

            # just use leaderpos for the location; wesnoth will always
            # recruit on the nearest adjacent tile
            if not wesnoth.recruit_unit(recruit,leaderpos): break

    def map_score(self,recruit):
        # calculate average speed in hexes/turn
        # and average defense in effective hp
        m = wesnoth.get_map()
        n = m.x * m.y

        speed = 0.0
        defense = 0.0
        for x in range(m.x):
            for y in range(m.y):
                loc = wesnoth.get_location(x,y)
                speed += 1.0 / recruit.movement_cost(loc)
                defense += 100.0 / recruit.defense_modifier(loc) - 1

        # speed is more important on larger maps
        speed *= self.mapsize * recruit.movement / n

        # scaled down because effective hp is over the lifetime of the unit,
        # while other scores are based on per-turn quantities
        defense *= HP_SCALE * recruit.hitpoints / n
        return speed,defense

    def combat_score(self,recruit):
        # combat advantage, in hp/turn, averaged over all enemy units
        tot = 0.0
        n = 0
        for loc,enem in wesnoth.get_units().iteritems():
            if not enem.is_enemy: continue
            n += 1
            tot += self.combat_advantage(recruit,enem)
            tot -= self.combat_advantage(enem,recruit)

        return tot/n

    def combat_advantage(self,attacker,defender):
        # combat advantage for attacker attacking defender
        best = 0.0
        for weapon in attacker.attacks():
            damage = weapon.damage * weapon.num_attacks * defender.damage_from(weapon) / 100.0

            best_retal = 0.0
            for retaliation in defender.attacks():
                if weapon.range == retaliation.range:
                    retal = retaliation.damage * retaliation.num_attacks * attacker.damage_from(retaliation) / 100.0
                    if retal > best_retal: best_retal = retal

            damage -= best_retal
            if damage > best: best = damage

        # scale down because not every attack hits
        return best/2

    def recruit_score(self,recruit):
        speed,defense = self.map_score(recruit)
        combat = self.combat_score(recruit)
        rval = (speed + defense + combat)/recruit.cost
        # only report "interesting" results
        if rval > 0:
            wesnoth.log_message("%s: (%.2f + %.2f + %.2f) / %d = %.3f" % (recruit.name,speed,defense,combat,recruit.cost,rval))
        return rval

    def do_one_move(self):
        enemlocs = wesnoth.get_enemy_destinations_by_unit().keys()
        self.enemdests = wesnoth.get_enemy_units_by_destination().keys()
        bestmove = (0,None,None,None) # score,orig,dest,target

        # find the best move
        for orig in wesnoth.get_destinations_by_unit().keys():
            # get a baseline score for this unit "standing pat"
            base_score = self.eval_move(orig,orig)
            for dest in wesnoth.get_destinations_by_unit()[orig]:
                # Bug workaround -- if we have recruited this turn,
                # get_destinations_by_unit() is incorrect
                if dest in wesnoth.get_units().keys() and dest != orig: continue
                score = self.eval_move(orig,dest) - base_score
                if score > bestmove[0]:
                    bestmove = (score,orig,dest,dest)
                for target in wesnoth.get_adjacent_tiles(dest):
                    if target in enemlocs:
                        fight = self.eval_fight(wesnoth.get_units()[orig],dest,target)+score
                        if orig == dest:
                            fight += STAND_NEXT_TO_ENEMY_PEN + NO_MOVE_PEN
                        else:
                            fight += NEXT_TO_ENEMY_PEN
                        if fight > bestmove[0]:
                            bestmove = (fight,orig,dest,target)

        if bestmove[1] == None:
            # no move improved the position, therefore we are done
            return False

        score,orig,dest,target = bestmove
        wesnoth.log_message("%.3f: %s->%s@%s"%(score,pos(orig),pos(dest),pos(target)))
        if dest != orig: wesnoth.move_unit(orig,dest)
        if dest in self.notmyvillages: self.notmyvillages.remove(dest)
        if target != dest: wesnoth.attack_unit(dest,target)

        return True

    def eval_fight(self,unit,dest,target):
        self.stats[1] += 1
        enem = wesnoth.get_units().get(target,None)
        if not enem: return 0

        # the base value for each unit:
        # I should give more weight to defeating a garrison
        unit_k = (LEVEL_SCORE*unit.type().level + BASE_UNIT_SCORE + LEADER_SCORE*unit.can_recruit\
            + FULL_XP_SCORE * unit.experience * 1.0 / unit.max_experience) * (BASE_COST_SCORE + unit.type().cost * COST_SCORE)
        enem_k = (LEVEL_SCORE*enem.type().level + BASE_UNIT_SCORE + LEADER_SCORE*enem.can_recruit\
            + FULL_XP_SCORE * enem.experience * 1.0 / enem.max_experience) * (BASE_COST_SCORE + enem.type().cost * COST_SCORE)

        unit_hp,enem_hp = unit.attack_statistics(dest,target)
        score = 0.0
        for hp,p in enem_hp.iteritems():
            score += p * (enem.hitpoints - hp) * enem_k / enem.max_hitpoints
            if hp<=0: score += p * enem_k
        for hp,p in unit_hp.iteritems():
            score -= p * (unit.hitpoints - hp) * unit_k / unit.max_hitpoints
            if hp<=0: score -= p * unit_k

        enem_xp = 8*enem.type().level
        if enem.type().level == 0:
            enem_xp = 4
        unit_xp = 8*unit.type().level
        if unit.type().level == 0:
            unit_xp = 4

        if enem.type().level >= unit.max_experience - unit.experience:
            for hp, p in unit_hp.iteritems():
                if hp > 0: score += LEVEL_CHANCE_BONUS * p * unit_k
        elif enem_xp >= unit.max_experience - unit.experience:
            for hp, p in enem_hp.iteritems():
                if hp <= 0: score += LEVEL_CHANCE_BONUS * p * unit_k
        if unit.type().level >= enem.max_experience - enem.experience:
            for hp, p in enem_hp.iteritems():
                if hp > 0: score -= LEVEL_CHANCE_BONUS * p * enem_k
        elif unit_xp >= enem.max_experience - enem.experience:
            for hp, p in unit_hp.iteritems():
                if hp <= 0: score += LEVEL_CHANCE_BONUS * p * enem_k

        return score

    def eval_move(self,orig,dest):
        enemlocs = wesnoth.get_enemy_destinations_by_unit().keys()
        self.stats[0] += 1
        score = 0.0

        unit = wesnoth.get_units().get(orig,None)
        if not unit: return
        unit_k = (LEVEL_SCORE*unit.type().level + BASE_UNIT_SCORE + LEADER_SCORE*unit.can_recruit\
            + FULL_XP_SCORE * unit.experience * 1.0 / unit.max_experience) * (BASE_COST_SCORE + unit.type().cost * COST_SCORE)

        # subtract 1 because terrain might be a factor
        speed = unit.type().movement - 1

        attackable=False
        if dest in self.enemdests:
            attackable = True
        else:
            for adj in wesnoth.get_adjacent_tiles(dest):
                if adj in self.enemdests:
                    attackable = True
                    break

        # capture villages
        if dest in self.notmyvillages:
            score += VILLAGE_SCORE
            for team in wesnoth.get_teams():
                if team.owns_village(dest) and team.is_enemy:
                    score += ENEMY_VILLAGE_BONUS

        bestdist=100
        if unit.can_recruit:
            # leader stays near keep
            for keep in self.keeps:
                dist=dest.distance_to(keep)
                if dist<bestdist:
                    bestdist=dist
                    if dist<=1: break
        else:
            # everyone else moves toward enemy leader
            for loc,enem in wesnoth.get_units().iteritems():
                if enem.is_enemy and enem.can_recruit:
                    dist=dest.distance_to(loc)
                    if dist<bestdist:
                        bestdist=dist
                        if dist<=1: break
        if bestdist > 1:
            for vil in self.notmyvillages:
                if dest==vil: continue
                dist=dest.distance_to(vil)
                if dist<bestdist:
                    bestdist=dist
                    if dist<=1: break
        score += (1.0 * speed) / (bestdist + speed)

        # healing
        # I am ignoring the value of healers, and regenerating units.  I don't think unit abilities
        # are correctly reported by the API, anyway.
        if (unit.poisoned or unit.hitpoints<unit.max_hitpoints) and wesnoth.get_map().is_village(dest):
            if unit.poisoned: healing = HEAL_POISON
            else:
                healing = unit.max_hitpoints-unit.hitpoints
                if healing > 8: healing = 8
            # reduce the healing bonus if we might get killed first
            if attackable: healing *= HEAL_ATTACKABLE
            score += HEAL_FACTOR * healing * unit_k / unit.max_hitpoints

        if attackable:
            # defense
            score -= unit.defense_modifier(dest) * DEFENSE_FACTOR

            # garrison
            if wesnoth.get_map().is_village(dest): score += GARRISON_SCORE

        # reduce chances of standing next to a unit without attacking for a whole turn
        if dest == orig:
            score -= NO_MOVE_PEN
            for target in wesnoth.get_adjacent_tiles(dest):
                if target in enemlocs:
                    score -= STAND_NEXT_TO_ENEMY_PEN
                    break
        else:
            for target in wesnoth.get_adjacent_tiles(dest):
                if target in enemlocs:
                    score -= NEXT_TO_ENEMY_PEN
                    break
            
        # end mod

        return score

ai = AI()
ai.recruit()
while 1:
    if not ai.do_one_move():
        break
ai.recruit()
ai.report_stats()
