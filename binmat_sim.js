function(context, args)
{
	var suit_t = {"FORM":0, "KIN":1, "DATA":2, "CHAOS":3, "VOID":4, "CHOICE":5, "COUNT_SUIT":6};
	var value_t = {"TWO":2, "THREE":3, "FOUR":4, "FIVE":5, "SIX":6, "SEVEN":7, "EIGHT":8, "NINE":9, "TEN":10, "TRAP":11, "WILD":12, "BOUNCE":13, "BREAK":14, "COUNT_VALUE":15};

	var card_strings = {2:"2",
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

	var piles =
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

	function card_make()
	{
		var card = new Object();

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

	function card_get_string(card)
	{
	    return card_strings[card.card_type]
	}

	function card_is_face_up(card)
	{
		return !card.face_down;
	}

	function card_manager_make()
	{
		var card_manager = new Object();

		card_manager.elems = []
		card_manager.card_fetch_counter = 0;

		for(var suit_offset = 0; suit_offset < 6; suit_offset++)
		{
			for(var card_offset = 0; card_offset < 13; card_offset++)
			{
				var c = card_make();

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
		var j, x, i;

		for (i = card_array.length - 1; i > 0; i--)
		{
			j = Math.floor(Math.random() * (i + 1));
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
		var card_list = new Object();

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
		for(var i=0; i < cl.cards.length; i++)
		{
			cl.cards[i].face_down = true;
		}
	}

	function card_list_make_cards_face_up(cl)
	{
        for(var i=0; i < cl.cards.length; i++)
		{
			cl.cards[i].face_down = false;
		}
	}

	function card_list_get_of_type(cl, type)
	{
		var ret = card_list_make();

		for(var i=0; i < cl.cards.length; i++)
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
		var sum = 0;

		for(var k2 = 0; k2 < cl.cards.length; k2++)
        {
			var current_card = cl.cards[k2];

            if(card_is_modifier(current_card))
                continue;

            sum += card_get_value(current_card);
        }

        var powers_to_bump_up = 0;

		for(var k1 = 0; k1 < cl.cards.length; k1++)
        {
			var c2 = cl.cards[k1];

            if(card_is_type(c2, value_t["WILD"]))
            {
                powers_to_bump_up++;
            }
        }

        for(var i=0; i < powers_to_bump_up; i++)
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
		for(var i = 0; i < cl.cards.length; i++)
		{
			if(!card_is_face_up(cl.cards[i]))
			{
				cl.cards.splice(i, 1);
				i--;
				continue;
			}
		}
	}

	function card_list_prune_to_face_down(cl)
	{
		for(var i = 0; i < cl.cards.length; i++)
		{
			if(card_is_face_up(cl.cards[i]))
			{
				cl.cards.splice(i, 1);
				i--;
				continue;
			}
		}
	}

	function card_list_only_top(cl)
	{
		var new_cl = card_list_make();

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

		var c = other.cards[other.cards.length-1];

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
		for(var i=0; i < cl.cards.length; i++)
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
		//for(var c of cl.cards)
		for(var k3 = 0; k3 < cl.cards.length; k3++)
		{
			var c = cl.cards[k3];

			if(card_is_type(c, type))
			{
				return true;
			}
		}

		return false;
	}

	function card_list_steal_all(cl, other)
	{
		//for(var c of other.cards)
		for(var counter = 0; counter < other.cards.length; counter++)
		{
			var c = other.cards[counter];

			cl.cards.push(c);
		}

		other.cards = []
	}

	function card_list_steal_all_of(cl, other, type)
	{
		for(var i=0; i < other.cards.length; i++)
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

	var piles_name =
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
		var lane = new Object();

		lane.card_piles = []

		for(var i=0; i < 4; i++)
		{
			lane.card_piles.push(card_list_make());
		}

		return lane;
	}

	var player_t = {"ATTACKER":0, "DEFENDER":1, "SPECTATOR":2, "OVERLORD":3, "REAL_STATE":4}

	var num_lanes = 6;

	function game_state_make()
	{
		var game_state = new Object();

		game_state.lanes = []
		game_state.piles = []
		game_state.turn = 0;

		for(var i=0; i < num_lanes; i++)
		{
			game_state.lanes.push(lane_make())
		}

		for(var i=0; i < 4; i++)
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

	function game_state_get_num_cards(gs, current_pile, lane)
	{
        return game_state_get_cards(gs, current_pile, lane).cards.length;
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
            var cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

            var ret = card_list_clone(cards);
            ret.cards = [];

            //for(var c of cards.cards)
			for(var card_count = 0; card_count < cards.cards.length; card_count++)
            {
				var c = cards.cards[card_count];

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
            var cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

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
            var cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

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
            var cards = card_list_clone(game_state_get_cards(gs, current_pile, lane));

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
		var cards = game_state_get_cards(gs, pile, lane);

		return card_list_is_face_down(cards);
	}

	function game_state_is_face_up(gs, pile, lane)
	{
		var cards = game_state_get_cards(gs, pile, lane);

		return card_list_is_face_up(cards);
	}

	function game_state_get_all_visible_cards(gs, player)
	{
		var ret = card_list_make();

		for(var i=0; i < piles["COUNT"]; i++)
		{
			if(piles_is_lane_type(i))
			{
				for(var lane = 0; lane < num_lanes; lane++)
				{
					var cards = game_state_get_visible_pile_cards_as(gs, i, player, lane);

					ret.cards.push.apply(ret.cards, cards.cards)
				}
			}
			else
			{
				var cards = game_state_get_visible_pile_cards_as(gs, i, player, -1);

				ret.cards.push.apply(ret.cards, cards.cards)
			}
		}

		return ret;
	}

	function game_state_get_pile_info(gs, pile, player, lane)
	{
		var num = game_state_get_cards(gs, pile, lane).cards.length;

		return {num:num};
	}

	function game_state_generate_new_game(gs, all_cards)
	{
		card_manager_reset_fetching(all_cards);

		var cards_in_lane = 13;

		shuffle_cards(all_cards.elems);

		for(var i = 0; i < piles["COUNT"]; i++)
		{
			if(piles_is_lane_type(i))
			{
				for(var lane = 0; lane < num_lanes; lane++)
				{
					var clist = game_state_get_cards(gs, i, lane);

					card_list_clear(clist);
				}
			}
			else
			{
				var clist = game_state_get_cards(gs, i, -1);

				card_list_clear(clist);
			}
		}

		for(var lane = 0; lane < num_lanes; lane++)
		{
			var cards = game_state_get_cards(gs, piles["LANE_DECK"], lane);

			for(var card_count = 0; card_count < cards_in_lane; card_count++)
			{
				cards.cards.push(card_manager_fetch_without_replacement(all_cards));
			}

			//for(var c of cards.cards)
			for(var to_clear = 0; to_clear < cards.cards.length; to_clear++)
			{
				var c = cards.cards[to_clear];

				c.face_down = true;
			}

			if(lane_has_faceup_top_card(lane))
			{
				cards.cards[cards.cards.length-1].face_down = false;
			}
		}

		gs.turn = 0;

		game_state_ensure_card_facing(gs);
	}

	function game_state_is_visible(gs, check, pile, player, lane)
	{
		var visible_cards = game_state_get_visible_pile_cards_as(gs, pile, player, lane);

		//for(var c of visible_cards.cards)
		for(var visible_count = 0; visible_count < visible_cards.cards.length; visible_count++)
		{
			var c = visible_cards.cards[visible_count];

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

	function game_state_get_visible_lane_stack_for_player(gs, player, lane)
	{
        if(player == player_t["ATTACKER"])
		{
			return game_state_get_visible_pile_cards_as(gs, piles["ATTACKER_STACK"], player, lane)
		}

		if(player == player_t["DEFENDER"])
		{
			return game_state_get_visible_pile_cards_as(gs, piles["DEFENDER_STACK"], player, lane);
		}
	}

	function game_state_transfer_top_card(gs, to_pile, to_lane, from_pile, from_lane)
	{
		var to_cards = game_state_get_cards(gs, to_pile, to_lane);
		var from_cards = game_state_get_cards(gs, from_pile, from_lane);

		return card_list_take_top_card(to_cards, from_cards).ok;
	}

	function game_state_lane_protected(gs, lane)
	{
		var defender_cards = game_state_get_cards(gs, piles["DEFENDER_STACK"], lane);

		return defender_cards.cards.length > 0;
	}

	function game_state_ensure_faceup_lanes(gs)
	{
		for(var i=0; i < num_lanes; i++)
		{
			if(lane_has_faceup_top_card(i))
			{
				var cards = game_state_get_cards(gs, piles["LANE_DECK"], i);

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

		card_list_make_cards_face_up(game_state_get_cards(gs, piles["ATTACKER_DISCARD"], -1));
	}

	function game_state_draw_from_impl(gs, pile, lane, player)
    {
        if(player != player_t["ATTACKER"] && player != player_t["DEFENDER"])
            return {ok:false};

        var hand = game_state_get_hand(gs, player);

        if(player == player_t["ATTACKER"])
        {
            if(pile == piles["ATTACKER_DECK"])
            {
                var success = game_state_transfer_top_card(gs, piles["ATTACKER_HAND"], -1, piles["ATTACKER_DECK"], -1);

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
                if(lane < 0 || lane >= num_lanes)
                    return {ok:false};

                if(game_state_lane_protected(gs, lane))
                    return {ok:false};

				var success = game_state_transfer_top_card(gs, piles["ATTACKER_HAND"], -1, piles["LANE_DECK"], lane);

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
                if(lane < 0 || lane >= num_lanes)
                    return {ok:false};

				var success = game_state_transfer_top_card(gs, piles["DEFENDER_HAND"], -1, piles["LANE_DECK"], lane);

                if(!success)
                {
					success = card_list_shuffle_in(game_state_get_cards(gs, piles["LANE_DECK"], lane), game_state_get_cards(gs, piles["LANE_DISCARD"], lane))

                    card_list_make_cards_face_down(game_state_get_cards(gs, piles["LANE_DECK"], lane));

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
		var result = game_state_draw_from_impl(gs, pile, lane, player);

		game_state_ensure_card_facing(gs);

		return result;
	}

	function game_state_can_play_face_up_on(gs, player, to_play, appropriate_stack)
	{
		if(card_is_type(to_play, value_t["BREAK"]) && !card_list_contains(appropriate_stack, value_t["BREAK"]) && appropriate_stack.cards.length > 0)
			return true;

        if(card_is_type(to_play, value_t["BREAK"]) && (card_list_contains(appropriate_stack, value_t["BREAK"]) || appropriate_stack.cards.length == 0))
			return false;

        if(player == player_t["ATTACKER"])
        {
            if(card_is_type(to_play, value_t["BOUNCE"]) && appropriate_stack.cards.length == 0)
                return true;

            if(card_is_type(to_play, value_t["BOUNCE"]) && appropriate_stack.cards.length > 0)
                return false;
        }
        if(player == player_t["DEFENDER"])
        {
            ///card_list being faceup has to have 1+ cards
            if(card_is_type(to_play, value_t["BOUNCE"]) && card_list_is_face_up(appropriate_stack))
                return true;

            if(card_is_type(to_play, value_t["BOUNCE"]) && !card_list_is_face_up(appropriate_stack))
                return false;
        }

        if(card_list_is_face_up(appropriate_stack))
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
	    var events = "";

		///copy
		var traps = card_list_get_of_type(my_stack, value_t["TRAP"]);

		//for(var c of traps.cards)
		for(var trap_num = 0; trap_num < traps.cards.length; trap_num++)
		{
			var c = traps.cards[trap_num];

			if(c.face_down)
			{
				c.face_down = false;

				var taken = card_list_take_top_card(discard, other_stack);

				if(taken.ok)
				{
                    events += "Trap Triggered and ate " + card_strings[taken.card.card_type] + "\n";

					taken.card.face_down = false;
				}
			}
		}

		return {events:events}
	}

	function game_state_trigger_combat_impl(gs, who_triggered, lane)
	{
		var attacker_stack = game_state_get_cards(gs, piles["ATTACKER_STACK"], lane);
		var defender_stack = game_state_get_cards(gs, piles["DEFENDER_STACK"], lane);

		defender_stack.face_up = true;

		var attacker_discard = game_state_get_cards(gs, piles["ATTACKER_DISCARD"], -1);
		var lane_discard = game_state_get_cards(gs, piles["LANE_DISCARD"], lane);

		var lane_deck = game_state_get_cards(gs, piles["LANE_DECK"], lane);

		var attacker_hand = game_state_get_cards(gs, piles["ATTACKER_HAND"], -1);

		var events = ""

		events += "Attacker Cards: "

		for(var aic = 0; aic < attacker_stack.cards.length; aic++)
        {
            events += card_get_string(attacker_stack.cards[aic]);
        }

        events += "\nDefender Cards: ";

        for(var dic = 0; dic < defender_stack.cards.length; dic++)
        {
            events += card_get_string(defender_stack.cards[dic]);
        }

        events += "\n";

		if(who_triggered == player_t["ATTACKER"])
		{
		    events += "Attacker Triggered\n";

			///attacker then defender
			events += game_state_process_trap_cards(gs, attacker_stack, defender_stack, attacker_discard).events;
			events += game_state_process_trap_cards(gs, defender_stack, attacker_stack, lane_discard).events;
		}

		if(who_triggered == player_t["DEFENDER"])
		{
		    events += "Defender Triggered\n";

			events += game_state_process_trap_cards(gs, defender_stack, attacker_stack, lane_discard).events;
			events += game_state_process_trap_cards(gs, attacker_stack, defender_stack, attacker_discard).events;
		}


		var should_bounce = false;

		var attacker_bounce = card_list_get_of_type(attacker_stack, value_t["BOUNCE"]);
		var defender_bounce = card_list_get_of_type(defender_stack, value_t["BOUNCE"]);

		if(attacker_bounce.cards.length > 0 || defender_bounce.cards.length > 0)
		{
			should_bounce = true;

			events += "Bounce due to Bounce card(s)\n";
		}

		//for(var c of attacker_stack.cards)
		for(var attacker_facedown_id = 0; attacker_facedown_id < attacker_stack.cards.length; attacker_facedown_id++)
		{
			var c = attacker_stack.cards[attacker_facedown_id];

			c.face_down = false;
		}

		//for(var c of defender_stack.cards)
		for(var defender_facedown_id = 0; defender_facedown_id < defender_stack.cards.length; defender_facedown_id++)
		{
			var c = defender_stack.cards[defender_facedown_id];

			c.face_down = false;
		}

		if(defender_bounce.cards.length > 0)
        {
            events += "Defender Bounce Card to Attacker Discard\n";
        }

        if(attacker_bounce.cards.length > 0)
        {
            events += "Attacker Bounce Card to Lane Discard\n";
        }

		card_list_steal_all_of(attacker_discard, defender_stack, value_t["BOUNCE"]);
		card_list_steal_all_of(lane_discard, attacker_stack, value_t["BOUNCE"]);

		var attacker_damage = card_list_calculate_stack_damage(attacker_stack);
		var defender_damage = card_list_calculate_stack_damage(defender_stack);

		events += "Attacker Damage: " + attacker_damage + "\n";
		events += "Defender Damage: " + defender_damage + "\n";

		if(attacker_damage == 0 && defender_damage == 0)
		{
			should_bounce = true;

			events += "0/0 Bounce\n";
		}

		if(should_bounce)
		{
		    events += "Bounce Resolved\n";

			card_list_steal_all(attacker_discard, attacker_stack);

			events += "Attacker Stack to Attacker Discard\n";

			return {ok:true, events:events}
		}

		if(attacker_damage < defender_damage)
		{
		    events += "Attacker did not win fight\n";

			card_list_steal_all(lane_discard, attacker_stack);

			events += "Attacker Stack to Lane Discard\n";

			return {ok:true, events:events};
		}

		var modify_combat_rules = card_list_get_of_type(attacker_stack, value_t["BREAK"]).cards.length > 0;

		var damage = (attacker_damage - defender_damage) + 1;

		if(modify_combat_rules)
		{
			damage = attacker_damage;

			events += "Using BREAK damage rules\n";
		}

		events += "Resolved Damage: " + damage + "\n";

		for(var cur_d = damage; cur_d > 0; cur_d--)
		{
			var cards_left = defender_stack.cards.length > 0;

			if(cards_left)
			{
				var taken = card_list_take_top_card(attacker_discard, defender_stack);

			    events += "Sent Defender Stack Card " + card_strings[taken.card.card_type] + " to Attacker Discard\n";

				continue;
			}

			var lane_cards_exist = lane_deck.cards.length > 0;

			if(lane_cards_exist)
			{
				var taken = card_list_take_top_card(attacker_hand, lane_deck);

				if(lane_has_faceup_top_card(lane))
                    events += "Sent Lane Deck Card " + card_strings[taken.card.card_type] + " to Attacker Hand\n";
                else
                    events += "Sent Lane Deck Card to attacker hand\n";

				continue;
			}

			var lane_discard_exists = lane_discard.cards.length > 0;

			if(lane_discard_exists)
			{
			    events += "Shuffled in Lane Discard\n";

				card_list_shuffle_in(lane_deck, lane_discard);
				var taken = card_list_take_top_card(attacker_hand, lane_deck);

				if(lane_has_faceup_top_card(lane))
                    events += "Sent Lane Deck Card " + card_strings[taken.card.card_type] + " to attacker hand\n";
                else
                    events += "Sent Lane Deck Card to attacker hand\n";

				continue;
			}

			card_list_steal_all(attacker_discard, attacker_stack);

			events += "Attacker WINS match\n";

			return {ok:true, win:true, events:events}
		}

		card_list_steal_all(attacker_discard, attacker_stack);

		events += "Sent Attacker Stack to Attacker Discard\n";

		return {ok:true, events:events}
	}

	function game_state_trigger_combat(gs, who_triggered, lane)
	{
	    var result = game_state_trigger_combat_impl(gs, who_triggered, lane);

	    game_state_ensure_card_facing(gs);

	    return result
	}

	function game_state_play_card_on_stack(gs, player, to_play, appropriate_stack, face_up, lane)
	{
		if(face_up && game_state_can_play_face_up_on(gs, player, to_play, appropriate_stack))
		{
			to_play.face_down = false;

			var is_face_down = card_list_is_face_down(appropriate_stack) || appropriate_stack.cards.length == 0;

			card_list_add_face_up_card(appropriate_stack, to_play);

			to_play.face_down = false;

			if(is_face_down)
			{
				var did_win = game_state_trigger_combat(gs, player, lane);

				to_play.face_down = false;

				return {ok:true, win:did_win.win, events:did_win.events}
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

		var hand = game_state_get_hand(gs, player);

		if(card_offset < 0 || card_offset >= hand.cards.length)
			return {ok:false};

		var to_play = hand.cards[card_offset];

		var appropriate_stack = game_state_get_lane_stack_for_player(gs, player, lane);

		var play_state = game_state_play_card_on_stack(gs, player, to_play, appropriate_stack, face_up, lane);

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

		var cards = game_state_get_lane_stack_for_player(gs, player, lane);

		if(cards.cards.length == 0)
			return {ok:false};

		var play_state = game_state_trigger_combat(gs, player, lane);

		return play_state;
	}

	function game_state_discard_hand_to_lane_discard(gs, lane, card_offset)
	{
		if(lane < 0 || lane >= num_lanes)
			return {ok:false};

		var lane_discard = game_state_get_cards(gs, piles["LANE_DISCARD"], lane);

		var hand = game_state_get_cards(gs, piles["DEFENDER_HAND"], lane);

		if(card_offset < 0 || card_offset >= hand.cards.length)
			return {ok:false};

		var c = hand.cards[card_offset];

		card_list_remove_card(hand, c);

		c.face_down = false;

		lane_discard.cards.push(c);

		return {ok:true};
	}

	function game_state_attacker_discard(gs, card_offset)
	{
        var discard = game_state_get_cards(gs, piles["ATTACKER_DISCARD"], -1);

		var hand = game_state_get_cards(gs, piles["ATTACKER_HAND"], -1);

		if(card_offset < 0 || card_offset >= hand.cards.length)
			return {ok:false};

		var c = hand.cards[card_offset];

		card_list_remove_card(hand, c);

		c.face_down = false;

		discard.cards.push(c);

		///draw 2 cards
        game_state_draw_from(gs, piles["ATTACKER_DECK"], -1, player_t["ATTACKER"]);
        game_state_draw_from(gs, piles["ATTACKER_DECK"], -1, player_t["ATTACKER"]);

		return {ok:true};
	}

	function is_defender_turn(gs)
	{
		return (gs.turn % 2) == 0;
	}

	function is_attacker_turn(gs)
	{
		return !is_defender_turn(gs);
	}

	function get_string()
	{
		return "String from JS";
	}

	function debug(gs)
	{
		var cards = game_state_get_cards(gs, piles["LANE_DECK"], 0);

		for(var i = 0; i < cards.cards.length; i++)
		{
			var c = cards.cards[i];

			print(c.card_type);
		}
	}

	function game_state_set_card(gs, pile, lane, card_offset, type, suit, face_down, card_list_face_up, size_of_card_list, turn)
	{
        var cards = game_state_get_cards(gs, pile, lane);

        for(var i=cards.cards.length; i < size_of_card_list; i++)
        {
            cards.cards.push(card_make());
        }

        cards.cards.length = size_of_card_list;

        cards.face_up = card_list_face_up;

        if(card_offset < cards.cards.length)
        {
            var card = cards.cards[card_offset];

            card.face_down = face_down;
            card.suit_type = suit;
            card.card_type = type;
        }

		gs.turn = turn;
	}

	function game_state_inc_turn(gs)
	{
        gs.turn = gs.turn + 1;
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

			game_state_lane_protected:game_state_lane_protected,
			game_state_get_num_cards:game_state_get_num_cards,
			game_state_get_visible_lane_stack_for_player:game_state_get_visible_lane_stack_for_player,

			game_state_attacker_discard:game_state_attacker_discard,

			game_state_get_hand:game_state_get_hand,

			get_string:get_string,
			debug:debug,

			game_state_inc_turn:game_state_inc_turn,

			game_state_set_card:game_state_set_card,

			card_list_calculate_stack_damage:card_list_calculate_stack_damage,
			card_list_prune_to_face_down:card_list_prune_to_face_down,
			card_list_clone:card_list_clone,
			card_list_accepts_face_down_cards:card_list_accepts_face_down_cards,

			piles_is_lane_type:piles_is_lane_type,
			piles:piles,
			piles_name:piles_name,
			player_t:player_t,
			value_t:value_t,
			card_strings:card_strings,
			is_defender_turn:is_defender_turn,
			is_attacker_turn:is_attacker_turn
			}

	//lane_make();
}
