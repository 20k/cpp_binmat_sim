function mainfunc(context, args)
{
	let suit_t = {"FORM":0, "KIN":1, "DATA":2, "CHAOS":3, "VOID":4, "CHOICE":5, "COUNT_SUIT":6}
	let value_t = {"TWO":2, "THREE":3, "FOUR":4, "FIVE":5, "SIX":6, "SEVEN":7, "EIGHT":8, "NINE":9, "TEN":10, "TRAP":11, "WILD":12, "BOUNCE":13, "BREAK":14, "COUNT_VALUE":15} 
	
	let card_strings = {2:"2",
        3:"3",
        4:"4",
        5:"5",
        6:"6",
        7:"7",
        8:"8",
        9:"9",
        10:"X",
        11:"@",
        12:"*",
        13:"?",
        14:">",
		15:"ERROR"}
	
	function card_make()
	{
		let card = new Object();
		
		card.face_down = false;
		card.suit_type = suit_t["COUNT_SUIT"];
		card.card_type = value_t["COUNT_VALUE"];
		
		return card
	}
	
	function card_set_from_offsets(card, suit_offset, card_offset)
	{
		card.suit_type = suit_offset;
		card.card_type = card_offset + value_t["TWO"]
	}
	
	function card_is_modifier(card)
	{
		return card.card_type >= value_t["TRAP"]
	}
	
	function card_is_type(card, type)
	{
		return card.card_type == type;
	}
	
	function card_get_value(card)
	{
		return card.card_type;
	}
	
	function card_is_face_up(card)
	{
		return !card.face_down;
	}
	
	function card_manager_make()
	{
		let card_manager = new Object();
		
		card_manager.elems = []
		card_manager.card_fetch_counter = 0;
		
		for(let suit_offset = 0; suit_offset < 6; suit_offset++)
		{
			for(let card_offset = 0; card_offset < 13; card_offset++)
			{
				let c = card_make();
				
				card_set_from_offsets(c, suit_offset, card_offset);
				
				card_manager.elems.push(c);
			}
		}
		
		return card_manager;
	}
	
	function card_manager_fetch_without_replacement(cm)
	{
		return cm.elems[cm.card_fetch_counter++]
	}
	
	function card_manager_reset_fetching(cm)
	{
		cm.card_fetch_counter = 0;
	}
	
	function shuffle_cards(card_array)
	{
		let j, x, i;
		
		for (i = card_array.length - 1; i > 0; i--) 
		{
			j = MATH.floor(MATH.random() * (i + 1));
			x = card_array[i];
			card_array[i] = card_array[j];
			card_array[j] = x;
		}
	}
	
	function is_power_of_2(x)
	{
		return x > 0 && !(x & (x-1));
	}
	
	function do_wild_roundup(sum)
	{
		if(sum == 0)
		{
			sum = 2;
			return sum;
		}

		if(is_power_of_2(sum))
		{
			sum = Math.pow(2, Math.log2(sum) + 1);
		}
		else
		{
			sum = Math.pow(2, Math.ceil(Math.log2(sum)));
		}

		return sum;
	}
	
	function card_list_make()
	{
		let card_list = new Object();
		
		card_list.cards = []
		card_list.face_up = false;
		
		return card_list;
	}
	
	function card_list_clear(cl)
	{
		cl.cards = [];
		cl.face_up = false;
	}
	
	function card_list_clone(cl)
	{
		return JSON.parse(JSON.stringify(cl));
	}
	
	function card_list_is_face_up(cl)
	{
		return cl.face_up && cl.cards.length > 0;
	}
	
	function card_list_is_face_down(cl)
	{
		return !cl.face_up && cl.cards.length > 0;
	}
	
	function card_list_accepts_face_down_cards(cl)
    {
        return card_list_is_face_down(cl) || cl.cards.length == 0;
    }
	
	function card_list_make_cards_face_down(cl)
	{
		for(let i=0; i < cl.cards.length; i++)
		{
			cl.cards[i].face_down = true;
		}
	}
	
	function card_list_get_of_type(cl, type)
	{
		let ret = card_list_make();
		
		for(let i=0; i < cl.cards.length; i++)
		{
			if(card_is_type(cl.cards[i], type))
			{
				ret.cards.push(cl.cards[i]);
			}
		}
		
		return ret;
	}
	
	function card_list_add_face_down_card(cl, card)
	{
		if(cl.cards.length == 0)
		{
			cl.face_up = false;
		}
		
		if(card_list_is_face_up(cl))
		{
			return false;
		}
		
		cl.cards.push(card);
	}
	
	function card_list_add_face_up_card(cl, card)
	{
		cl.face_up = true;
		
		cl.cards.push(card);
	}
	
	function card_list_calculate_stack_damage(cl)
	{
		let sum = 0;

        for(let c1 of cl.cards)
        {
            if(card_is_modifier(c1))
                continue;

            sum += card_get_value(c1);
        }

        let powers_to_bump_up = 0;

        for(let c2 of cl.cards)
        {
            if(card_is_type(c2, "WILD"))
            {
                powers_to_bump_up++;
            }
        }

        for(let i=0; i < powers_to_bump_up; i++)
        {
            ///so. If sum is 7, should get rounded up to 8
            ///if 8, get rounded up to 16 etc

            sum = do_wild_roundup(sum);
        }

        if(is_power_of_2(sum))
        {
            return Math.log2(sum);
        }
        else
        {
            return 0;
        }
	}
	
	function card_list_prune_to_face_up(cl)
	{
		for(let i = 0; i < cl.cards.length; i++)
		{
			if(!card_is_face_up(cl.cards[i]))
			{
				cl.cards.splice(i, 1);
				i--;
				continue;
			}
		}
	}
	
	function card_list_only_top(cl)
	{
		let new_cl = card_list_make();
		
		if(cl.cards.length > 0)
		{
			new_cl.cards.push(cl.cards[cl.cards.length-1])
		}	
		
		return new_cl
	}
	
	function card_list_take_top_card(cl, other)
	{
		if(other.cards.length == 0)
		{
			return {ok:false}
		}
		
		let c = other.cards[other.cards.length-1];
		
		other.cards.pop();
		
		cl.cards.push(c);
		
		return {ok:true, card:c}
	}
	
	function card_list_shuffle_in(cl, other)
	{
		if(other.cards.length == 0)
			return false;
		
		cl.cards.push.apply(cl.cards, other.cards)
		
		shuffle_cards(cl);
		
		other.cards = [];
		
		return true;
	}
	
	function card_list_remove_card(cl, card)
	{
		for(let i=0; i < cl.cards.length; i++)
        {
            if(cl.cards[i] == card)
            {
                cl.cards.splice(i, 1);
                return true;
            }
        }

        return false;
	}
	
	function card_list_contains(cl, type)
	{
		for(let c of cl.cards)
		{
			if(card_is_type(c, type))
			{
				return true;
			}
		}
		
		return false;
	}
	
	function card_list_steal_all(cl, other)
	{
		for(let c of other.cards)
		{
			cl.cards.push(c);
		}
		
		other.cards = []
	}
	
	function card_list_steal_all_of(cl, other, type)
	{
		for(let i=0; i < other.cards.length; i++)
		{
			if(card_is_type(other.cards[i], type))
			{
				cl.cards.push(other.cards[i]);
				
				other.cards.splice(i, 1);
				i--;
				continue;
			}
		}
	}
	
	let piles = 
	{
		"LANE_DISCARD":0,
		"LANE_DECK":1,
		"DEFENDER_STACK":2,
		"ATTACKER_STACK":3,

        "ATTACKER_DECK":4,
        "ATTACKER_DISCARD":5,
        "ATTACKER_HAND":6,
        "DEFENDER_HAND":7,
        "COUNT":8
	}
	
	let piles_name = 
	[
		"Lane Discard",
		"Lane Deck",
		"Defender Stack",
		"Attacker Stack",
		
		"Attacker Deck",
		"Attacker Discard",
		"Attacker Hand",
		"Defender Hand"		
	];
	
	function piles_is_lane_type(p)
	{
		return p == piles["LANE_DISCARD"] || p == piles["LANE_DECK"] || p == piles["DEFENDER_STACK"] || p == piles["ATTACKER_STACK"];
	}
	
	function lane_make()
	{
		let lane = new Object();
		
		lane.card_piles = []
		
		for(let i=0; i < 4; i++)
		{
			lane.card_piles.push(card_list_make());
		}
	
		return lane;
	}
	
	let player_t = {"ATTACKER":0, "DEFENDER":1, "SPECTATOR":2, "OVERLORD":3, "REAL_STATE":4}
	
	let num_lanes = 6;
	
	function game_state_make()
	{
		let game_state = new Object();
		
		game_state.lanes = []
		game_state.piles = []
		game_state.turn = 0;
		
		for(let i=0; i < num_lanes; i++)
		{
			game_state.lanes.push(lane_make())
		}
		
		for(let i=0; i < 4; i++)
		{
			game_state.piles.push(card_list_make())
		}
		
		return game_state;
	}
	
	function game_state_get_cards(gs, current_pile, lane)
	{
		if(current_pile == piles["ATTACKER_DECK"])
            return gs.piles[0];

        if(current_pile == piles["ATTACKER_DISCARD"])
            return gs.piles[1];

        if(current_pile == piles["ATTACKER_HAND"])
            return gs.piles[2];

        if(current_pile == piles["DEFENDER_HAND"])
            return gs.piles[3];

        if(current_pile == piles["LANE_DISCARD"])
            return gs.lanes[lane].card_piles[0];

        if(current_pile == piles["LANE_DECK"])
            return gs.lanes[lane].card_piles[1];

        if(current_pile == piles["DEFENDER_STACK"])
            return gs.lanes[lane].card_piles[2];

        if(current_pile == piles["ATTACKER_STACK"])
            return gs.lanes[lane].card_piles[3];
	}
	
	function game_state_reseat_cards(gs, current_pile, lane, new_cards)
	{
		if(current_pile == piles["ATTACKER_DECK"])
            gs.piles[0] = new_cards;

        if(current_pile == piles["ATTACKER_DISCARD"])
            gs.piles[1] = new_cards;

        if(current_pile == piles["ATTACKER_HAND"])
            gs.piles[2] = new_cards;

        if(current_pile == piles["DEFENDER_HAND"])
            gs.piles[3] = new_cards;

        if(current_pile == piles["LANE_DISCARD"])
            gs.lanes[lane].card_piles[0] = new_cards;

        if(current_pile == piles["LANE_DECK"])
            gs.lanes[lane].card_piles[1] = new_cards;

        if(current_pile == piles["DEFENDER_STACK"])
            gs.lanes[lane].card_piles[2] = new_cards;

        if(current_pile == piles["ATTACKER_STACK"])
            gs.lanes[lane].card_piles[3] = new_cards;
	}

    function lane_has_faceup_top_card(lane)
    {
        return lane >= 3;
    }

	function game_state_get_visible_pile_cards_as(gs, current_pile, player, lane)
	{		
		if(player == player_t["OVERLORD"])
        {
            return card_list_clone(game_state_get_cards(gs, current_pile, lane));
        }

        if(player == player_t["REAL_STATE"])
        {
            let cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

            let ret = card_list_clone(cards);
            ret.cards = [];

            for(let c of cards.cards)
            {
                if(card_is_face_up(c))
                {
                    ret.cards.push(c);
                }
            }

            return ret;
        }
		
        if(current_pile == piles["ATTACKER_DECK"])
        {
            return card_list_make();
        }

        if(current_pile == piles["ATTACKER_DISCARD"])
        {
            return card_list_clone(game_state_get_cards(gs, current_pile, -1));
        }

        if(current_pile == piles["ATTACKER_HAND"] && player != player_t["DEFENDER"])
        {
            return card_list_clone(game_state_get_cards(gs, current_pile, -1));
        }

        if(current_pile == piles["DEFENDER_HAND"] && player != player_t["ATTACKER"])
        {
            return card_list_clone(game_state_get_cards(gs, current_pile, -1));
        }

        if(current_pile == piles["LANE_DECK"])
        {			
            let cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

            if(!lane_has_faceup_top_card(lane))
                return card_list_make();

            if(lane_has_faceup_top_card(lane))
                return card_list_only_top(cards);
        }

        if(current_pile == piles["LANE_DISCARD"])
        {
            return card_list_clone(game_state_get_cards(gs, current_pile, lane));
        }

        if(current_pile == piles["DEFENDER_STACK"])
        {
            ///important that this is a copy
            let cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

            if(player != player_t["ATTACKER"])
                return cards;

            if(player != player_t["DEFENDER"])
            {
                card_list_prune_to_face_up(cards);
                return cards;
            }
        }

        if(current_pile == piles["ATTACKER_STACK"])
        {
            let cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

            if(player != player_t["DEFENDER"])
                return cards;

            if(player != player_t["ATTACKER"])
            {
                card_list_prune_to_face_up(cards);
                return cards;
            }
        }
		
        return card_list_make();
	}
	
	function game_state_is_face_down(gs, pile, lane)
	{
		let cards = game_state_get_cards(gs, pile, lane);
		
		return card_list_is_face_down(cards);
	}
	
	function game_state_is_face_up(gs, pile, lane)
	{
		let cards = game_state_get_cards(gs, pile, lane);
		
		return card_list_is_face_up(cards);
	}
	
	function game_state_get_all_visible_cards(gs, player)
	{
		let ret = card_list_make();
		
		for(let i=0; i < piles["COUNT"]; i++)
		{
			if(piles_is_lane_type(i))
			{
				for(let lane = 0; lane < num_lanes; lane++)
				{
					let cards = game_state_get_visible_pile_cards_as(gs, i, player, lane);
					
					ret.cards.push.apply(ret.cards, cards.cards)
				}
			}
			else
			{
				let cards = game_state_get_visible_pile_cards_as(gs, i, player, -1);
				
				ret.cards.push.apply(ret.cards, cards.cards)
			}
		}
		
		return ret;
	}
	
	function game_state_get_pile_info(gs, pile, player, lane)
	{
		let num = game_state_get_cards(gs, pile, lane).cards.length;
		
		return {num:num};
	}
	
	function game_state_generate_new_game(gs, all_cards)
	{
		card_manager_reset_fetching(all_cards);
		
		let cards_in_lane = 13;
		
		shuffle_cards(all_cards.elems);
		
		for(let i = 0; i < piles["COUNT"]; i++)
		{
			if(piles_is_lane_type(i))
			{
				for(let lane = 0; lane < num_lanes; lane++)
				{
					let clist = game_state_get_cards(gs, i, lane);
					
					card_list_clear(clist);
				}
			}
			else
			{
				let clist = game_state_get_cards(gs, i, -1);
				
				card_list_clear(clist);
			}
		}
		
		for(let lane = 0; lane < num_lanes; lane++)
		{
			let cards = game_state_get_cards(gs, piles["LANE_DECK"], lane);
			
			for(let card_count = 0; card_count < cards_in_lane; card_count++)
			{
				cards.cards.push(card_manager_fetch_without_replacement(all_cards));
			}
						
			for(let c of cards.cards)
			{				
				c.face_down = true;
			}
			
			if(lane_has_faceup_top_card(lane))
			{
				cards.cards[cards.cards.length-1].face_down = false;
			}
		}
		
		game_state_ensure_card_facing(gs);
	}
	
	function game_state_is_visible(gs, check, pile, player, lane)
	{
		let visible_cards = game_state_get_visible_pile_cards_as(gs, pile, player, lane);
		
		for(let c of visible_cards.cards)
		{
			if(c == check)
				return true;
		}
		
		return false;
	}
	
	function game_state_get_hand(gs, player)
	{
		if(player == player_t["ATTACKER"])
		{
			return game_state_get_cards(gs, piles["ATTACKER_HAND"], -1)
		}
		
		if(player == player_t["DEFENDER"])
		{
			return game_state_get_cards(gs, piles["DEFENDER_HAND"], -1);
		}
	}
	
	function game_state_get_lane_stack_for_player(gs, player, lane)
	{
		if(player == player_t["ATTACKER"])
		{
			return game_state_get_cards(gs, piles["ATTACKER_STACK"], lane)
		}
		
		if(player == player_t["DEFENDER"])
		{
			return game_state_get_cards(gs, piles["DEFENDER_STACK"], lane);
		}
	}
	
	function game_state_transfer_top_card(gs, to_pile, to_lane, from_pile, from_lane)
	{
		let to_cards = game_state_get_cards(gs, to_pile, to_lane);
		let from_cards = game_state_get_cards(gs, from_pile, from_lane);
		
		return card_list_take_top_card(to_cards, from_cards).ok;
	}
	
	function game_state_lane_protected(gs, lane)
	{
		let defender_cards = game_state_get_cards(gs, piles["DEFENDER_STACK"], lane);
		
		return defender_cards.cards.length > 0;
	}
	
	function game_state_ensure_faceup_lanes(gs)
	{
		for(let i=0; i < num_lanes; i++)
		{
			if(lane_has_faceup_top_card(i))
			{
				let cards = game_state_get_cards(gs, piles["LANE_DECK"], i);
				
				if(cards.cards.length > 0)
				{
					cards.cards[cards.cards.length-1].face_down = false;
				}
			}
		}
	}
	
	function game_state_ensure_facedown_cards(gs)
	{
		card_list_make_cards_face_down(game_state_get_cards(gs, piles["ATTACKER_HAND"], -1));
		card_list_make_cards_face_down(game_state_get_cards(gs, piles["DEFENDER_HAND"], -1));
		
		card_list_make_cards_face_down(game_state_get_cards(gs, piles["ATTACKER_DECK"], -1));
	}
	
	function game_state_ensure_card_facing(gs)
	{
		game_state_ensure_faceup_lanes(gs);
		game_state_ensure_facedown_cards(gs);
	}
	
	function game_state_draw_from_impl(gs, pile, lane, player)
    {
        if(player != player_t["ATTACKER"] && player != player_t["DEFENDER"])
            return {ok:false};

        let hand = game_state_get_hand(gs, player);

        if(player == player_t["ATTACKER"])
        {
            if(pile == piles["ATTACKER_DECK"])
            {
                let success = game_state_transfer_top_card(gs, piles["ATTACKER_HAND"], -1, piles["ATTACKER_DECK"], -1);

                if(!success)
                {
					success = card_list_shuffle_in(game_state_get_cards(gs, piles["ATTACKER_DECK"], -1), game_state_get_cards(gs, piles["ATTACKER_DISCARD"], -1))
					
					card_list_make_cards_face_down(game_state_get_cards(gs, piles["ATTACKER_DECK"], -1));
					
                    if(success)
                    {
						game_state_transfer_top_card(gs, piles["ATTACKER_HAND"], -1, piles["ATTACKER_DECK"], -1);
					}

                    return {ok:success};
                }

                return {ok:true};
            }

            if(pile == piles["LANE_DECK"])
            {
                if(game_state_lane_protected(gs, lane))
                    return {ok:false};

				let success = game_state_transfer_top_card(gs, piles["ATTACKER_HAND"], -1, piles["LANE_DECK"], lane);
				
                if(!success)
                {
					success = card_list_shuffle_in(game_state_get_cards(gs, piles["LANE_DECK"], lane), game_state_get_cards(gs, piles["LANE_DISCARD"], lane))
					
					card_list_make_cards_face_down(game_state_get_cards(gs, piles["LANE_DECK"], lane));
					
                    if(success)
                    {
						game_state_transfer_top_card(gs, piles["ATTACKER_HAND"], -1, piles["LANE_DECK"], lane);
                    }
                    else
                    {		
						///attacker wins
						return {ok:success, win:true} 
                    }

                    return {ok:success};
                }

                return {ok:true};
            }
        }

        if(player == player_t["DEFENDER"])
        {
            if(pile == piles["LANE_DECK"])
            {
				let success = game_state_transfer_top_card(gs, piles["DEFENDER_HAND"], -1, piles["LANE_DECK"], lane);
				
                if(!success)
                {
					success = card_list_shuffle_in(game_state_get_cards(gs, piles["LANE_DECK"], lane), game_state_get_cards(gs, piles["LANE_DISCARD"], lane))
					
                    card_list_make_cards_face_down(game_state_get_cards(gs, piles["LANE_DECK"], -1));

                    if(success)
                    {
						game_state_transfer_top_card(gs, piles["DEFENDER_HAND"], -1, piles["LANE_DECK"], lane);
					}
                    else
                    {
						///attacker wins
                        return {ok:success, win:true}
                    }

                    return {ok:success};
                }

                return {ok:true};
            }
        }

        return {ok:false};
    }
	
	function game_state_draw_from(gs, pile, lane, player)
	{
		if(lane < 0 || lane >= 6)
			return {ok:false}
		
		let result = game_state_draw_from_impl(gs, pile, lane, player);
		
		game_state_ensure_card_facing(gs);
		
		return result;
	}
	
	function game_state_can_play_face_up_on(gs, player, to_play, appropriate_stack)
	{
		if(card_is_type(to_play, value_t["BREAK"]) && !card_list_contains(appropriate_stack, value_t["BREAK"]) && appropriate_stack.cards.length > 0)
			return true;
		
		if(card_is_type(to_play, value_t["BOUNCE"]) && player == player_t["ATTACKER"] && appropriate_stack.cards.length == 0)
			return true;
	
		return false;
	}
	
	function game_state_can_play_face_down_on(gs, player, to_play, appropriate_stack)
    {
		if(!card_list_accepts_face_down_cards(appropriate_stack))
            return false;

        ///no 2 breaks in a stack
		if(card_is_type(to_play, value_t["BREAK"]) && card_list_contains(appropriate_stack, value_t["BREAK"]))
            return false;

        ///cannot ever play a break on an empty stack        
		if(card_is_type(to_play, value_t["BREAK"]) && appropriate_stack.cards.length == 0)
			return false;

        return true;
    }
	
	function game_state_process_trap_cards(gs, my_stack, other_stack, discard)
	{
		///copy
		let traps = card_list_get_of_type(my_stack, value_t["TRAP"]);
		
		for(let c of traps.cards)
		{
			if(c.face_down)
			{
				c.face_down = false;
				
				let taken = card_list_take_top_card(discard, other_stack);
				
				if(taken.ok)
				{
					taken.card.face_down = true;
				}
			}
		}
	}
	
	function game_state_trigger_combat(gs, who_triggered, lane)
	{
		let attacker_stack = game_state_get_cards(gs, piles["ATTACKER_STACK"], lane);
		let defender_stack = game_state_get_cards(gs, piles["DEFENDER_STACK"], lane);
		
		defender_stack.face_up = true;
		
		let attacker_discard = game_state_get_cards(gs, piles["ATTACKER_DISCARD"], -1);
		let lane_discard = game_state_get_cards(gs, piles["LANE_DISCARD"], lane);
		
		let lane_deck = game_state_get_cards(gs, piles["LANE_DECK"], lane);
		
		let attacker_hand = game_state_get_cards(gs, piles["ATTACKER_HAND"], -1);
				
		if(who_triggered == player_t["ATTACKER"])
		{				
			///attacker then defender
			game_state_process_trap_cards(gs, attacker_stack, defender_stack, attacker_discard);
			game_state_process_trap_cards(gs, defender_stack, attacker_stack, lane_discard);
		}
		
		if(who_triggered == player_t["DEFENDER"])
		{			
			game_state_process_trap_cards(gs, defender_stack, attacker_stack, lane_discard);
			game_state_process_trap_cards(gs, attacker_stack, defender_stack, attacker_discard);
		}
		
		
		let should_bounce = false;
		
		let attacker_bounce = card_list_get_of_type(attacker_stack, value_t["BOUNCE"]);
		let defender_bounce = card_list_get_of_type(defender_stack, value_t["BOUNCE"]);
		
		if(attacker_bounce.cards.length > 0 || defender_bounce.cards.length > 0)
		{
			should_bounce = true;
		}
		
		for(let c of attacker_stack.cards)
		{
			c.face_down = false;
		}
		
		for(let c of defender_stack.cards)
		{
			c.face_down = false;
		}
		
		card_list_steal_all_of(attacker_stack, defender_stack, value_t["BOUNCE"]);
		card_list_steal_all_of(lane_discard, attacker_stack, value_t["BOUNCE"]);
		
		let attacker_damage = card_list_calculate_stack_damage(attacker_stack);
		let defender_damage = card_list_calculate_stack_damage(defender_stack);
		
		if(attacker_damage == 0 && defender_damage == 0)
		{
			should_bounce = true;
		}
		
		if(should_bounce)
		{
			card_list_steal_all(attacker_discard, attacker_stack);
			
			return {ok:true}
		}
		
		if(attacker_damage < defender_damage)
		{
			card_list_steal_all(lane_discard, attacker_stack);
			return {ok:true};
		}
		
		let modify_combat_rules = card_list_get_of_type(attacker_stack, value_t["BREAK"]).cards.length > 0;
		
		let damage = (attacker_damage - defender_damage) + 1;
		
		if(modify_combat_rules)
		{
			damage = attacker_damage;
		}
		
		for(let cur_d = damage; cur_d > 0; cur_d--)
		{
			let cards_left = defender_stack.cards.length > 0;
			
			if(cards_left)
			{
				card_list_take_top_card(attacker_discard, defender_stack);
				continue;
			}
			
			let lane_cards_exist = lane_deck.cards.length > 0;
			
			if(lane_cards_exist)
			{
				card_list_take_top_card(attacker_hand, lane_deck);
				continue;
			}
			
			let lane_discard_exists = lane_discard.cards.length > 0;
			
			if(lane_discard_exists)
			{
				card_list_shuffle_in(lane_deck, lane_discard);
				card_list_take_top_card(attacker_hand, lane_deck);
				continue;
			}
			
			return {ok:true, win:true}
		}
		
		card_list_steal_all(attacker_discard, attacker_stack);
		
		return {ok:true}
	}
	
	function game_state_play_card_on_stack(gs, player, to_play, appropriate_stack, face_up, lane)
	{		
		if(face_up && game_state_can_play_face_up_on(gs, player, to_play, appropriate_stack))
		{
			to_play.face_down = false;
			
			let is_face_down = card_list_is_face_down(appropriate_stack) || appropriate_stack.cards.length == 0;
			
			card_list_add_face_up_card(appropriate_stack, to_play);
			
			if(is_face_down)
			{				
				let did_win = game_state_trigger_combat(gs, player, lane);
				
				return {ok:true, win:did_win.win}
			}
			
			return {ok:true};
		}
		
		if(!face_up && game_state_can_play_face_down_on(gs, player, to_play, appropriate_stack))
		{
			to_play.face_down = true;
			
			card_list_add_face_down_card(appropriate_stack, to_play);
			
			return {ok:true};
		}
		
		return {ok:false};
	}
	
	function game_state_play_to_stack_from_hand(gs, player, lane, card_offset, face_up)
	{
		if(lane < 0 || lane >= num_lanes)
			return {ok:false}
		
		let hand = game_state_get_hand(gs, player);

		if(card_offset < 0 || card_offset >= hand.cards.length)
			return {ok:false};
		
		let to_play = hand.cards[card_offset];
		
		let appropriate_stack = game_state_get_lane_stack_for_player(gs, player, lane);
		
		let play_state = game_state_play_card_on_stack(gs, player, to_play, appropriate_stack, face_up, lane);
			
		if(play_state.ok)
		{
			card_list_remove_card(hand, to_play);
		}
		
		return play_state;
	}
	
	function game_state_try_trigger_combat(gs, player, lane)
	{
		if(lane < 0 || lane >= num_lanes)
			return {ok:false};
		
		let cards = game_state_get_lane_stack_for_player(gs, player, lane);
		
		if(cards.cards.length == 0)
			return {ok:false};
		
		let play_state = game_state_trigger_combat(gs, player, lane);
		
		return play_state;
	}
	
	function game_state_discard_hand_to_lane_discard(gs, lane, card_offset)
	{
		if(lane < 0 || lane >= num_lanes)
			return {ok:false};
		
		let lane_discard = game_state_get_cards(piles["LANE_DISCARD"], lane);
		
		let hand = game_state_get_cards(piles["DEFENDER_HAND"], lane);
		
		if(card_offset < 0 || card_offset >= hand.cards.length)
			return {ok:false};
		
		let c = hand.cards[card_offset];
		
		card_list_remove_card(hand, c);
		
		c.face_down = false;
		
		lane_discard.cards.push(c);
		
		return {ok:true};
	}
	
	/*function game_exists_in_db(game_id)
	{
		var result = #db.f({game_id:game_id}).first();

		if(result)
			return true;
		
		return false;
	}
	
	function save_users_to_db(game_id, attacker, defender)
	{				
		var res = #db.f({buser:game_id}).first();
		
		if(res)
			return;
		
		#db.i({buser:game_id, attacker:attacker, defender:defender});		
	}
	
	function is_attacker(game_id, user)
	{
		var res = #db.f({buser:game_id}).first();
		
		if(!res)
			return false;
		
		return res.attacker == user;
	}	
	
	function is_defender(game_id, user)
	{
		var res = #db.f({buser:game_id}).first();
		
		if(!res)
			return false;
		
		return res.defender == user;
	}
	
	function get_attacker(game_id)
	{
		var res = #db.f({buser:game_id}).first();
		
		if(!res)
			return "";
		
		return res.attacker;
	}
	
	function get_defender(game_id)
	{
		var res = #db.f({buser:game_id}).first();
		
		if(!res)
			return "";
		
		return res.defender;
	}
	
	function save_state_to_db(game_id, gs)
	{
		if(game_exists_in_db(game_id))
		{
			#db.r({game_id:game_id});
		}
		
		// for(let i=0; i < piles["COUNT"]; i++)
		// {
			// if(piles_is_lane_type(i))
			// {
				// for(let lane = 0; lane < 6; lane++)
				// {
					// let cards = game_state_get_cards(gs, i, lane);				
					
					// #db.i({game_id:game_id, pile:i, lane:lane, data:cards});
				// }
			// }
			// else
			// {
				// let cards = game_state_get_cards(gs, i, -1);
				
				// #db.i({game_id:game_id, pile:i, lane:-1, data:cards});
			// }
		// }
		
		#db.i({game_id:game_id, state:gs});
	}
	
	function load_state_from_db(game_id)
	{
		if(!game_exists_in_db(game_id))
			return null;
		
		let gs = game_state_make();
				
		// for(let i=0; i < piles["COUNT"]; i++)
		// {
			// if(piles_is_lane_type(i))
			// {
				// for(let lane = 0; lane < 6; lane++)
				// {					
					// let cards = #db.f({game_id:game_id, pile:i, lane:lane}).first().data;
					
					// game_state_reseat_cards(gs, i, lane, cards);
				// }
			// }
			// else
			// {
				// let cards = #db.f({game_id:game_id, pile:i, lane:-1}).first().data;
				
				// game_state_reseat_cards(gs, i, -1, cards);
			// }
		// }
		
		// return gs;
		
		gs = #db.f({game_id:game_id}).first().state;
		
		if(!gs.turn)
			gs.turn = 0;
		
		return gs
	}*/
	
	function is_defender_turn(gs)
	{
		return (gs.turn % 2) == 0;
	}
	
	function is_attacker_turn(gs)
	{
		return !is_defender_turn(gs);
	}
	
	return {
			card_manager_make:card_manager_make,
			game_state_make:game_state_make, 
			game_state_generate_new_game:game_state_generate_new_game,
			game_state_draw_from:game_state_draw_from, 
			game_state_play_to_stack_from_hand:game_state_play_to_stack_from_hand,
			game_state_try_trigger_combat:game_state_try_trigger_combat,
			game_state_discard_hand_to_lane_discard:game_state_discard_hand_to_lane_discard,
			game_state_get_all_visible_cards:game_state_get_all_visible_cards,
			game_state_get_visible_pile_cards_as:game_state_get_visible_pile_cards_as,
			game_state_get_pile_info:game_state_get_pile_info,
			
			game_state_get_cards:game_state_get_cards,
			card_list_get_of_type:card_list_get_of_type,
			
			game_state_is_face_down:game_state_is_face_down,
			game_state_is_face_up:game_state_is_face_up,
			
			//save_state_to_db:save_state_to_db,
			//load_state_from_db:load_state_from_db,
			piles_is_lane_type:piles_is_lane_type,
			piles:piles,
			piles_name:piles_name,
			player_t:player_t,
			value_t:value_t,
			card_strings:card_strings,
			//save_users_to_db:save_users_to_db,
			//is_attacker:is_attacker,
			//is_defender:is_defender,
			//get_attacker:get_attacker,
			//get_defender:get_defender,
			is_defender_turn:is_defender_turn,
			is_attacker_turn:is_attacker_turn
			}
	
	//lane_make();
}
