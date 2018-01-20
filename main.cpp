#include <iostream>
#include <SFML/Graphics.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>
#include <imgui/imgui_internal.h>
#include <vec/vec.hpp>
#include "../serialise/serialise.hpp"
#include "manager.hpp"
#include <math.h>
#include "font_renderer.hpp"
//#include "network_updater.hpp"
#include "../4space_server/networking.hpp"

#define CARD_WIDTH 25
#define CARD_HEIGHT CARD_WIDTH * 1.7

#include "js_interop.hpp"

struct tooltip
{
    static std::string current;

    static void add(const std::string& line)
    {
        current += line + "\n";
    }
};

///screw u windows
#undef VOID

std::string tooltip::current;

struct card : basic_entity
{
    bool face_down = false;

    enum suit_t
    {
        FORM = 0,
        KIN = 1,
        DATA = 2,
        CHAOS = 3,
        VOID = 4,
        CHOICE = 5,
        COUNT_SUIT
    };

    suit_t suit_type = suit_t::COUNT_SUIT;

    enum value_t
    {
        TWO = 2,
        THREE = 3,
        FOUR = 4,
        FIVE = 5,
        SIX = 6,
        SEVEN = 7,
        EIGHT = 8,
        NINE = 9,
        TEN = 10,

        TRAP = 11,
        WILD = 12,
        BOUNCE = 13,
        BREAK = 14,
        COUNT_VALUE = 15
    };

    std::vector<std::string> strings
    {
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "10",
        "@",
        "*",
        "?",
        ">",
        "ERROR",
    };

    value_t card_type = value_t::TWO;

    virtual void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(face_down, ser);
        s.handle_serialise(suit_type, ser);
        s.handle_serialise(card_type, ser);
    }

    bool operator==(card& other)
    {
        return suit_type == other.suit_type && card_type == other.card_type;
    }

    void set_card_from_offsets(int suit_offset, int card_offset)
    {
        assert(suit_offset >= 0 && suit_offset < COUNT_SUIT);
        assert(card_offset >= 0 && (card_offset < COUNT_VALUE - TWO));

        suit_type = (suit_t)suit_offset;
        card_type = (value_t)(card_offset + TWO);
    }

    bool is_modifier()
    {
        return card_type >= TRAP;
    }

    /*bool is_normal()
    {
        return !is_modifier();
    }*/

    bool is_type(value_t val)
    {
        return card_type == val;
    }

    int get_value()
    {
        assert(!is_modifier());

        return (int)card_type;
    }

    bool is_face_up()
    {
        return !face_down;
    }

    bool is_visible()
    {
        return is_face_up();
    }

    std::string get_string()
    {
        return strings[(int)(card_type - TWO)];
    }

    bool is_within(vec2f pos)
    {
        vec2f centre = info.pos;

        vec2f tl = centre - (vec2f){CARD_WIDTH, CARD_HEIGHT}/2.f;
        vec2f br = centre + (vec2f){CARD_WIDTH, CARD_HEIGHT}/2.f;

        return pos.x() >= tl.x() && pos.y() >= tl.y() && pos.x() < br.x() && pos.y() < br.y();
    }

    void render_card_background(sf::RenderWindow& win, bool should_render_face_up)
    {
        vec3f face_up_col = {1,1,1};
        vec3f face_down_col = {0.5f, 0.5f, 0.5f};

        sf::RectangleShape shape;

        shape.setPosition(info.pos.x(), info.pos.y());

        vec2f dim = {CARD_WIDTH, CARD_HEIGHT};

        shape.setSize(sf::Vector2f(dim.x(), dim.y()));

        shape.setOrigin(dim.x()/2.f, dim.y()/2.f);

        vec3f col = {1,1,1};

        col = should_render_face_up ? face_up_col : face_down_col;

        col = col * 255.f;

        shape.setFillColor(sf::Color(col.x(), col.y(), col.z()));

        win.draw(shape);
    }

    static void render_empty(vec2f pos, sf::RenderWindow& win)
    {
        vec3f empty_pile_col = {1,1,1};

        vec3f col = empty_pile_col * 255.f;

        sf::RectangleShape shape;
        shape.setPosition(pos.x(), pos.y());

        vec2f dim = {CARD_WIDTH, CARD_HEIGHT};

        shape.setSize(sf::Vector2f(dim.x(), dim.y()));
        shape.setOrigin(dim.x()/2.f, dim.y()/2.f);

        shape.setOutlineThickness(3);
        shape.setOutlineColor(sf::Color(col.x(), col.y(), col.z()));

        //shape.setFillColor(sf::Color(col.x(), col.y(), col.z()));

        shape.setFillColor(sf::Color(0,0,0,0));

        win.draw(shape);
    }

    void render_face_down(sf::RenderWindow& win)
    {
        render_card_background(win, false);
    }

    void render_face_up(sf::RenderWindow& win)
    {
        render_card_background(win, true);

        render_font(win, get_string(), info.pos, {1,0,0,1});
    }

    void render(sf::RenderWindow& win) override
    {
        if(is_face_up())
            render_face_up(win);
        else
            render_face_down(win);
    }

    /*bool is_hidden()
    {
        return !face_up && hidden;
    }*/
};

struct card_manager : manager<card>
{
    void generate_cards()
    {
        for(int suit_offset = 0; suit_offset < 6; suit_offset++)
        {
            for(int card_offset = 0; card_offset < 13; card_offset++)
            {
                card c;

                c.set_card_from_offsets(suit_offset, card_offset);
            }
        }
    }

    int card_fetch_counter = 0;

    /*card& fetch_without_replacement()
    {
        return elems[card_fetch_counter++];
    }*/

    void reset_fetching()
    {
        card_fetch_counter = 0;
    }
};

void shuffle_cards(std::vector<card>& cards)
{
    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(cards.begin(), cards.end(), g);
}

bool is_power_of_2(int x)
{
    return x > 0 && !(x & (x-1));
}

int do_wild_roundup(int sum)
{
    if(sum == 0)
    {
        sum = 2;
        return sum;
    }

    if(is_power_of_2(sum))
    {
        sum = pow(2, log2(sum) + 1);
    }
    else
    {
        sum = pow(2, ceil(log2(sum)));
    }

    return sum;
}

struct card_list : serialisable
{
    ///left is bottom, right is top
    std::vector<card> cards;

    bool face_up = false;

    arg_idx javascript_id = arg_idx(-1);

    trans info;

    bool is_within(vec2f pos)
    {
        vec2f centre = info.pos;

        vec2f tl = centre - (vec2f){CARD_WIDTH, CARD_HEIGHT}/2.f;
        vec2f br = centre + (vec2f){CARD_WIDTH, CARD_HEIGHT}/2.f;

        return pos.x() >= tl.x() && pos.y() >= tl.y() && pos.x() < br.x() && pos.y() < br.y();
    }

    virtual void do_serialise(serialise& s, bool ser)
    {
        s.handle_serialise(cards, ser);
        s.handle_serialise(face_up, ser);
    }

    bool is_face_up()
    {
        return face_up && cards.size() > 0;
    }

    bool is_face_down()
    {
        return !face_up && cards.size() > 0;
    }

    bool accepts_face_down_cards()
    {
        return is_face_down() || cards.size() == 0;
    }

    card_list get_of_type(card::value_t type)
    {
        card_list ret;

        for(card& c : cards)
        {
            if(c.is_type(type))
            {
                ret.cards.push_back(c);
            }
        }

        return ret;
    }

    bool add_face_down_card(card& c)
    {
        if(cards.size() == 0)
        {
            face_up = false;
        }

        if(is_face_up())
            return false;

        cards.push_back(c);
    }

    void add_face_up_card(card& c)
    {
        face_up = true;

        cards.push_back(c);
    }

    int calculate_stack_damage()
    {
        int sum = 0;

        for(card& c : cards)
        {
            if(c.is_modifier())
                continue;

            sum += c.get_value();
        }

        int powers_to_bump_up = 0;

        for(card& c : cards)
        {
            if(c.is_type(card::WILD))
            {
                powers_to_bump_up++;
            }
        }

        for(int i=0; i < powers_to_bump_up; i++)
        {
            ///so. If sum is 7, should get rounded up to 8
            ///if 8, get rounded up to 16 etc

            sum = do_wild_roundup(sum);
        }

        if(is_power_of_2(sum))
        {
            return log2(sum);
        }
        else
        {
            return 0;
        }
    }

    void prune_to_face_up()
    {
        for(int i=0; i < cards.size(); i++)
        {
            if(!cards[i].is_face_up())
            {
                cards.erase(cards.begin() + i);
                i--;
                continue;
            }
        }
    }

    card_list only_top()
    {
        card_list ncl;

        if(cards.size() > 0)
        {
            ncl.cards.push_back(cards.back());
        }

        return ncl;
    }

    std::optional<card> take_top_card(card_list& other)
    {
        if(other.cards.size() == 0)
        {
            return std::nullopt;
        }

        card& c = other.cards.back();

        other.cards.pop_back();

        cards.push_back(c);

        return c;
    }

    bool shuffle_in(card_list& other)
    {
        if(other.cards.size() == 0)
            return false;

        cards.insert(cards.end(), other.cards.begin(), other.cards.end());

        shuffle_cards(cards);

        other.cards.clear();

        return true;
    }

    bool remove_card(card& c)
    {
        for(int i=0; i < cards.size(); i++)
        {
            if(cards[i] == c)
            {
                cards.erase(cards.begin() + i);
                return true;
            }
        }

        return false;
    }

    bool contains(card::value_t type)
    {
        for(card& c : cards)
        {
            if(c.is_type(type))
                return true;
        }

        return false;
    }

    void steal_all(card_list& other)
    {
        for(card& c : other.cards)
        {
            cards.push_back(c);
        }

        other.cards.clear();
    }

    void steal_all_of(card_list& other, card::value_t type)
    {
        for(int i=0; i < other.cards.size(); i++)
        {
            if(other.cards[i].is_type(type))
            {
                cards.push_back(other.cards[i]);

                other.cards.erase(other.cards.begin() + i);
                i--;
                continue;
            }
        }
    }
};

namespace piles
{
    enum piles_t
    {
        LANE_DISCARD,
        LANE_DECK,
        DEFENDER_STACK,
        ATTACKER_STACK,

        ATTACKER_DECK,
        ATTACKER_DISCARD,
        ATTACKER_HAND,
        DEFENDER_HAND,
        COUNT,
    };

    inline
    bool is_lane_type(piles_t p)
    {
        return p == LANE_DISCARD || p == LANE_DECK || p == DEFENDER_STACK || p == ATTACKER_STACK;
    }
}

struct lane : serialisable
{
    ///lane discard, lane deck, defender stack, attacker stack
    std::array<card_list, 4> card_piles;

    virtual void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(card_piles, ser);
    }
};

struct game_state : serialisable
{
    std::array<lane, 6> lanes;

    ///Attacker deck, attacker discard, attacker hand, defender hand
    std::array<card_list, 4> piles;

    int turn = 0;

    bool cleanup = false;

    virtual void do_serialise(serialise& s, bool ser) override
    {
        s.handle_serialise(lanes, ser);
        s.handle_serialise(piles, ser);
        s.handle_serialise(turn, ser);
    }

    enum player_t
    {
        ATTACKER,
        DEFENDER,
        SPECTATOR,
        OVERLORD, ///can see everything
        REAL_STATE, ///sees real face up/down status of cards
    };

    card_list& get_cards(piles::piles_t current_pile, int lane = -1)
    {
        if(current_pile == piles::ATTACKER_DECK)
            return piles[0];

        if(current_pile == piles::ATTACKER_DISCARD)
            return piles[1];

        if(current_pile == piles::ATTACKER_HAND)
            return piles[2];

        if(current_pile == piles::DEFENDER_HAND)
            return piles[3];

        assert(lane >= 0 && lane < 6);

        if(current_pile == piles::LANE_DISCARD)
            return lanes[lane].card_piles[0];

        if(current_pile == piles::LANE_DECK)
            return lanes[lane].card_piles[1];

        if(current_pile == piles::DEFENDER_STACK)
            return lanes[lane].card_piles[2];

        if(current_pile == piles::ATTACKER_STACK)
            return lanes[lane].card_piles[3];
    }

    bool lane_has_faceup_top_card(int lane)
    {
        return lane >= 3;
    }

    card_list get_visible_pile_cards_as(piles::piles_t current_pile, player_t player, int lane = -1)
    {
        using namespace piles;

        if(player == OVERLORD)
        {
            return get_cards(current_pile, lane);
        }

        if(player == REAL_STATE)
        {
            card_list cards = get_cards(current_pile, lane);

            card_list ret = cards;
            ret.cards.clear();

            for(card& c : cards.cards)
            {
                if(c.is_face_up())
                {
                    ret.cards.push_back(c);
                }
            }

            return ret;
        }

        if(current_pile == ATTACKER_DECK)
        {
            return card_list();
        }

        if(current_pile == ATTACKER_DISCARD)
        {
            return get_cards(current_pile, -1);
        }

        if(current_pile == ATTACKER_HAND && player != DEFENDER)
        {
            return get_cards(current_pile, -1);
        }

        if(current_pile == DEFENDER_HAND && player != ATTACKER)
        {
            return get_cards(current_pile, -1);
        }

        if(current_pile == LANE_DECK)
        {
            card_list cards = get_cards(current_pile, lane);

            if(!lane_has_faceup_top_card(lane))
                return card_list();

            if(lane_has_faceup_top_card(lane))
                return cards.only_top();
        }

        if(current_pile == LANE_DISCARD)
        {
            return get_cards(current_pile, lane);
        }

        if(current_pile == DEFENDER_STACK)
        {
            ///important that this is a copy
            card_list cards = get_cards(current_pile, lane);

            if(player != ATTACKER)
                return cards;

            if(player != DEFENDER)
            {
                cards.prune_to_face_up();
                return cards;
            }
        }

        if(current_pile == ATTACKER_STACK)
        {
            card_list cards = get_cards(current_pile, lane);

            if(player != DEFENDER)
                return cards;

            if(player != ATTACKER)
            {
                cards.prune_to_face_up();
                return cards;
            }
        }

        return card_list();
    }

    #define NUM_LANES 6

    card_list get_all_visible_cards(player_t player)
    {
        card_list ret;

        for(int i=0; i < piles::COUNT; i++)
        {
            if(is_lane_type((piles::piles_t)i))
            {
                for(int lane = 0; lane < NUM_LANES; lane++)
                {
                    card_list cards = get_visible_pile_cards_as((piles::piles_t)i, player, lane);

                    ret.cards.insert(ret.cards.end(), cards.cards.begin(), cards.cards.end());
                }
            }
            else
            {
                card_list cards = get_visible_pile_cards_as((piles::piles_t)i, player, -1);

                ret.cards.insert(ret.cards.end(), cards.cards.begin(), cards.cards.end());
            }
        }

        return ret;
    }


    bool is_visible(card& check, piles::piles_t pile, player_t player, int lane)
    {
        card_list visible_cards = get_visible_pile_cards_as(pile, player, lane);

        for(card& c : visible_cards.cards)
        {
            if(c == check)
                return true;
        }

        return false;
    }

    void render_row(piles::piles_t pile, int cnum, int max_num, vec2f centre, player_t player, sf::RenderWindow& win, float yoffset)
    {
        card_list& current_deck = get_cards(pile, cnum);

        float card_separation = CARD_WIDTH * 1.4f;

        float lane_x = (card_separation * cnum) - (card_separation * max_num/2.f) + centre.x();

        vec2f real_pos = {lane_x, centre.y() + yoffset};

        current_deck.info.pos = real_pos;

        for(card& c : current_deck.cards)
        {
            c.info.pos = real_pos;

            if(is_visible(c, pile, player, cnum))
            {
                c.render_face_up(win);
            }
            else
            {
                c.render_face_down(win);
            }
        }

        if(current_deck.cards.size() == 0)
        {
            card::render_empty(real_pos, win);
            return;
        }

        vec2f br = {lane_x, centre.y() + yoffset};

        br += (vec2f){CARD_WIDTH/2.f, CARD_HEIGHT/2.f};

        if(current_deck.cards.size() > 0)
            render_font(win, std::to_string(current_deck.cards.size()), br, {1,1,1,1}, 0.6f);
    }

    void render_individual(piles::piles_t pile, int cnum, int max_num, vec2f centre, player_t player, sf::RenderWindow& win, float yoffset)
    {
        card_list& current_deck = get_cards(pile, -1);

        float card_separation = CARD_WIDTH * 1.4f;

        float lane_x = (card_separation * cnum) - (card_separation * max_num/2.f) + centre.x();

        vec2f real_pos = {lane_x, centre.y() + yoffset};

        current_deck.info.pos = real_pos;

        if(cnum < 0 || cnum >= current_deck.cards.size())
        {
            card::render_empty(real_pos, win);
            return;
        }

        card& c = current_deck.cards[cnum];

        c.info.pos = real_pos;

        if(is_visible(c, pile, player, cnum))
        {
            c.render_face_up(win);
        }
        else
        {
            c.render_face_down(win);
        }
    }

    void render(sf::RenderWindow& win, player_t player)
    {
        vec2f dim = {win.getSize().x, win.getSize().y};

        vec2f centre = dim/2.f;

        float vertical_sep = CARD_WIDTH * 2.5f;

        for(int i=0; i < NUM_LANES; i++)
        {
            render_row(piles::LANE_DECK, i, NUM_LANES, centre, player, win, 0.f);

            render_row(piles::LANE_DISCARD, i, NUM_LANES, centre, player, win, vertical_sep);

            render_row(piles::DEFENDER_STACK, i, NUM_LANES, centre, player, win, -vertical_sep);

            render_row(piles::ATTACKER_STACK, i, NUM_LANES, centre, player, win, -2*vertical_sep);
        }

        card_list defender_hand = get_cards(piles::DEFENDER_HAND, -1);

        for(int i=0; i < defender_hand.cards.size(); i++)
        {
            render_individual(piles::DEFENDER_HAND, i, defender_hand.cards.size(), centre, player, win, vertical_sep * 4);
        }

        card_list attacker_hand = get_cards(piles::ATTACKER_HAND, -1);

        for(int i=0; i < attacker_hand.cards.size(); i++)
        {
            render_individual(piles::ATTACKER_HAND, i, attacker_hand.cards.size(), centre, player, win, -vertical_sep * 5);
        }

        render_individual(piles::ATTACKER_DECK, 0, 1, {centre.x() + CARD_WIDTH * 5.4, centre.y()}, player, win, -vertical_sep * 4);

        render_individual(piles::ATTACKER_DISCARD, 0, 1, {centre.x() + CARD_WIDTH * 4, centre.y()}, player, win, -vertical_sep * 4);
    }

    player_t viewer = REAL_STATE;

    void set_viewer_state(player_t player)
    {
        viewer = player;
    }

    card_list build(stack_duk& sd, arg_idx card_list_id)
    {
        card_list ret;

        arg_idx face_up_id = sd.get_prop_string(card_list_id, "face_up");

        bool is_face_up = duk_get_boolean(sd.ctx, -1);

        arg_idx cards_id = sd.get_prop_string(card_list_id, "cards");

        int array_length = sd.get_length(cards_id);

        for(int kk=0; kk < array_length; kk++)
        {
            card c;

            arg_idx card_id = sd.get_prop_index(cards_id, kk);

            arg_idx face_down_id = sd.get_prop_string(card_id, "face_down");
            arg_idx suit_type_id = sd.get_prop_string(card_id, "suit_type");
            arg_idx card_type_id = sd.get_prop_string(card_id, "card_type");

            c.suit_type = (card::suit_t)sd.get_int(suit_type_id);
            c.card_type = (card::value_t)sd.get_int(card_type_id);

            c.face_down = sd.get_boolean(face_down_id);

            sd.pop_n(4);

            ret.cards.push_back(c);
        }

        sd.pop_n(2);

        ret.javascript_id = card_list_id;
        ret.face_up = is_face_up;

        return ret;
    }

    void import_state_from_js(stack_duk& sd, arg_idx gs_id)
    {
        arg_idx turns_id = sd.get_prop_string(gs_id, "turn");

        turn = duk_get_int(sd.ctx, -1);

        arg_idx piles_id = sd.get_prop_string(gs_id, "piles");

        int num_piles = sd.get_length(piles_id);

        assert(num_piles == 4);

        for(int i=0; i < num_piles; i++)
        {
            arg_idx card_list_id = sd.get_prop_index(piles_id, i);

            card_list found = build(sd, card_list_id);

            piles[i] = found;

            sd.pop_n(1);
        }

        sd.pop_n(2);

        arg_idx lanes_id = sd.get_prop_string(gs_id, "lanes");

        int found_lanes = sd.get_length(lanes_id);

        assert(found_lanes == 6);

        for(int i=0; i < found_lanes; i++)
        {
            arg_idx specific_lane = sd.get_prop_index(lanes_id, i);

            arg_idx card_list_id = sd.get_prop_string(specific_lane, "card_piles");

            int num_in_lane = sd.get_length(card_list_id);

            assert(num_in_lane == 4);

            for(int kk=0; kk < num_in_lane; kk++)
            {
                arg_idx specific_card_list = sd.get_prop_index(card_list_id, kk);

                card_list found = build(sd, specific_card_list);

                lanes[i].card_piles[kk] = found;

                sd.pop_n(1);
            }

            sd.pop_n(2);

        }

        sd.pop_n(1);
    }

    bool attacker_turn(stack_duk& sd, arg_idx gs_id)
    {
        arg_idx id = call_function_from_absolute(sd, "is_attacker_turn", gs_id);

        bool valid = sd.get_boolean(id);

        sd.pop_n(1);

        return valid;
    }

    bool defender_turn(stack_duk& sd, arg_idx gs_id)
    {
        arg_idx id = call_function_from_absolute(sd, "is_defender_turn", gs_id);

        bool valid = sd.get_boolean(id);

        sd.pop_n(1);

        return valid;
    }

    void set_card(stack_duk& sd, arg_idx gs_id, piles::piles_t pile, int lane, int card_offset, card& c, int card_list_face_up)
    {
        call_function_from_absolute(sd, "game_state_set_card", gs_id, (int)pile, lane, card_offset, (int)c.card_type, (int)c.suit_type, (bool)c.face_down, (bool)card_list_face_up);

        sd.pop_n(1);
    }

    void propagate_lane_decks_to_js(stack_duk& sd, arg_idx gs_id)
    {
        piles::piles_t pile = piles::LANE_DECK;

        printf("propagating\n");

        for(int lane=0; lane < NUM_LANES; lane++)
        {
            card_list& cards = get_cards(pile, lane);

            printf("in lane %i\n", cards.cards.size());

            for(int c=0; c < cards.cards.size(); c++)
            {
                set_card(sd, gs_id, pile, lane, c, cards.cards[c], cards.face_up);

                if(!cards.cards[c].face_down)
                {
                    std::cout << "fup " << cards.cards[c].get_string() << std::endl;
                }
            }
        }
    }


    bool can_act(player_t player, stack_duk& sd, arg_idx gs_id)
    {
        if(player == DEFENDER && defender_turn(sd, gs_id))
            return true;

        if(player == ATTACKER && attacker_turn(sd, gs_id))
            return true;

        return false;
    }

    card_list& get_hand(player_t player)
    {
        if(player == ATTACKER)
        {
            return get_cards(piles::ATTACKER_HAND, -1);
        }

        if(player == DEFENDER)
        {
            return get_cards(piles::DEFENDER_HAND, -1);
        }

        printf("bad player in get hand\n");

        assert(false);
    }
};

void tests()
{
    assert(do_wild_roundup(0) == 2);
    assert(do_wild_roundup(2) == 4);
    assert(do_wild_roundup(1) == 2);
    assert(do_wild_roundup(7) == 8);
    assert(do_wild_roundup(8) == 16);
    assert(do_wild_roundup(9) == 16);
    assert(do_wild_roundup(16) == 32);
    assert(do_wild_roundup(17) == 32);
}

struct command : serialisable
{
    bool cleanup = false;

    enum type
    {
        ATTACK_DRAW_LANE,
        ATTACK_DRAW_DECK,
        ATTACK_PLAY_STACK,
        ATTACK_INITIATE_COMBAT,

        DEFENDER_DRAW_LANE,
        DEFENDER_PLAY_STACK,
        DEFENDER_DISCARD_TO,

        PASS,

        COMMAND_COUNT
    };

    type to_exec = COMMAND_COUNT;

    piles::piles_t pile = piles::COUNT;
    int lane_selected = 0;
    game_state::player_t player = game_state::REAL_STATE;
    int is_faceup = false;
    int hand_card_offset = 0;

    void execute(stack_duk& sd, arg_idx gs_id)
    {
        if(to_exec == ATTACK_DRAW_LANE)
        {
            call_function_from_absolute(sd, "game_state_draw_from", gs_id, (int)piles::LANE_DECK, lane_selected, (int)game_state::ATTACKER);
        }

        if(to_exec == ATTACK_DRAW_DECK)
        {
            call_function_from_absolute(sd, "game_state_draw_from", gs_id, (int)piles::ATTACKER_DECK, -1, (int)game_state::ATTACKER);
        }

        if(to_exec == ATTACK_PLAY_STACK)
        {
            call_function_from_absolute(sd, "game_state_play_to_stack_from_hand", gs_id, (int)game_state::ATTACKER, lane_selected, hand_card_offset, is_faceup);
        }


        if(to_exec == ATTACK_INITIATE_COMBAT)
        {
            call_function_from_absolute(sd, "game_state_try_trigger_combat", gs_id, (int)game_state::ATTACKER, lane_selected);
        }



        if(to_exec == DEFENDER_DRAW_LANE)
        {
            call_function_from_absolute(sd, "game_state_draw_from", gs_id, (int)piles::LANE_DECK, lane_selected, (int)game_state::DEFENDER);
        }

        if(to_exec == DEFENDER_PLAY_STACK)
        {
            call_function_from_absolute(sd, "game_state_play_to_stack_from_hand", gs_id, (int)game_state::DEFENDER, lane_selected, hand_card_offset, is_faceup);
        }

        if(to_exec == DEFENDER_DISCARD_TO)
        {
            call_function_from_absolute(sd, "game_state_discard_hand_to_lane_discard", gs_id, lane_selected, hand_card_offset);
        }

        if(to_exec == PASS)
        {

        }

        call_function_from_absolute(sd, "game_state_inc_turn", gs_id);
    }

    virtual void do_serialise(serialise& s, bool ser)
    {
        s.handle_serialise(to_exec, ser);
        s.handle_serialise(pile, ser);
        s.handle_serialise(lane_selected, ser);
        s.handle_serialise(player, ser);
        s.handle_serialise(is_faceup, ser);
        s.handle_serialise(hand_card_offset, ser);
    }
};

struct seamless_ui_state
{
    int selected_card_id = -1;
    piles::piles_t pile = piles::COUNT;
    int lane = -1;

    bool card_selected = false;
    bool pile_selected = false;

    bool initiate_combat = false;
    int is_faceup = false;

    command get_command(game_state::player_t player)
    {
        command c;
        c.pile = pile;
        c.lane_selected = lane;
        c.hand_card_offset = selected_card_id;
        c.player = player;
        c.is_faceup = is_faceup;

        if(initiate_combat)
        {
            c.to_exec = command::ATTACK_INITIATE_COMBAT;
            return c;
        }

        if(pile == piles::LANE_DECK)
        {
            if(player == game_state::ATTACKER)
                c.to_exec = command::ATTACK_DRAW_LANE;
            if(player == game_state::DEFENDER)
                c.to_exec = command::DEFENDER_DRAW_LANE;
        }

        if(pile == piles::ATTACKER_DECK)
        {
            c.to_exec = command::ATTACK_DRAW_DECK;
        }

        if(pile == piles::LANE_DISCARD)
        {
            c.to_exec = command::DEFENDER_DISCARD_TO;
        }

        if(pile == piles::ATTACKER_STACK)
        {
            c.to_exec = command::ATTACK_PLAY_STACK;
        }

        if(pile == piles::DEFENDER_STACK)
        {
            c.to_exec = command::DEFENDER_PLAY_STACK;
        }

        return c;
    }
};

struct command_manager : serialisable
{
    bool cleanup = false;

    std::vector<command> commands;

    virtual void do_serialise(serialise& s, bool ser)
    {
        s.handle_serialise_no_clear(commands, ser);
    }

    void exec_all(stack_duk& sd, arg_idx gs_id, seamless_ui_state& ui_state)
    {
        if(commands.size() > 0)
            ui_state = seamless_ui_state();

        for(command& c : commands)
        {
            c.execute(sd, gs_id);
        }

        commands.clear();
    }

    bool should_network = false;

    void network(network_state& net_state)
    {
        if(!should_network)
            return;

        std::vector<command_manager*> me_hack{this};

        update_strategy net_update;
        net_update.do_update_strategy(1.f, 0.0, me_hack, net_state, 0);

        should_network = false;
    }

    void add(const command& c)
    {
        commands.push_back(c);
        should_network = true;
    }
};

///hmm. Maybe instead of networking state we should just network commands
void do_ui(stack_duk& sd, arg_idx gs_id, command_manager& commands, game_state::player_t player, game_state& basic_state)
{
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ///ATTACKER ACTIONS:
    ///Draw card from lane deck if not protected
    ///Draw card from attacker deck
    ///Play a card to one of 6 attacker stacks
    ///Initiate combat in one of 6 attacker stacks


    ///DEFENDER ACTIONS:
    ///Draw card from lane deck
    ///Play a card to one of 6 defender stacks
    ///Place card in the discard pile of a lane from your hand

    ///MISC ACTIONS:
    ///Inspect lane discard piles
    ///Inspect attacker discard pile
    ///Inspect any attacker stacks
    ///Inspect any face up defender stack
    ///Inspect own hand

    /*static int lane_selected = 0;
    static int is_faceup = false;
    static int hand_card_offset = 0;*/

    static command to_exec;

    ImGui::InputInt("Lane", &to_exec.lane_selected);
    ImGui::InputInt("Face Up?", &to_exec.is_faceup);
    ImGui::InputInt("Hand Card Number", &to_exec.hand_card_offset);

    to_exec.is_faceup = clamp(to_exec.is_faceup, 0, 1);
    //hand_card_offset = clamp(hand_card_offset, 0, current);

    to_exec.lane_selected = clamp(to_exec.lane_selected, 0, 6);

    ImGui::End();

    bool update = false;

    if((player == game_state::ATTACKER && basic_state.attacker_turn(sd, gs_id)) || player == game_state::OVERLORD)
    {
        ImGui::Begin("Attacker Actions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::Button("Draw from Lane Deck"))
        {
            //call_function_from_absolute(sd, "game_state_draw_from", gs_id, (int)piles::LANE_DECK, lane_selected, (int)game_state::ATTACKER);

            to_exec.to_exec = command::ATTACK_DRAW_LANE;
            to_exec.pile = piles::LANE_DECK;
            to_exec.player = game_state::ATTACKER;

            update = true;
        }

        if(ImGui::Button("Draw From Attacker Deck"))
        {
            //call_function_from_absolute(sd, "game_state_draw_from", gs_id, (int)piles::ATTACKER_DECK, -1, (int)game_state::ATTACKER);

            to_exec.to_exec = command::ATTACK_DRAW_DECK;
            to_exec.pile = piles::ATTACKER_DECK;
            to_exec.player = game_state::ATTACKER;

            update = true;
        }

        if(ImGui::Button("Play to Stack"))
        {
            //call_function_from_absolute(sd, "game_state_play_to_stack_from_hand", gs_id, (int)game_state::ATTACKER, lane_selected, hand_card_offset, is_faceup);

            to_exec.to_exec = command::ATTACK_PLAY_STACK;
            to_exec.player = game_state::ATTACKER;

            update = true;
        }

        if(ImGui::Button("Initiate Combat At Lane"))
        {
            //call_function_from_absolute(sd, "game_state_try_trigger_combat", gs_id, (int)game_state::ATTACKER, lane_selected);

            to_exec.to_exec = command::ATTACK_INITIATE_COMBAT;
            to_exec.player = game_state::ATTACKER;

            update = true;
        }

        if(ImGui::Button("Pass"))
        {
            to_exec.to_exec = command::PASS;
            to_exec.player = game_state::ATTACKER;
            update = true;
        }

        ImGui::End();
    }

    if((player == game_state::DEFENDER && basic_state.defender_turn(sd, gs_id)) || player == game_state::OVERLORD)
    {
        ImGui::Begin("Defender Actions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if(ImGui::Button("Draw From Lane Deck"))
        {
            //call_function_from_absolute(sd, "game_state_draw_from", gs_id, (int)piles::LANE_DECK, lane_selected, (int)game_state::DEFENDER);

            to_exec.to_exec = command::DEFENDER_DRAW_LANE;
            to_exec.pile = piles::LANE_DECK;
            to_exec.player = game_state::DEFENDER;

            update = true;
        }

        if(ImGui::Button("Play to Stack"))
        {
            //call_function_from_absolute(sd, "game_state_play_to_stack_from_hand", gs_id, (int)game_state::DEFENDER, lane_selected, hand_card_offset, is_faceup);

            to_exec.to_exec = command::DEFENDER_PLAY_STACK;
            to_exec.player = game_state::DEFENDER;

            update = true;
        }

        if(ImGui::Button("Discard Card to Lane Discard Pile"))
        {
            //call_function_from_absolute(sd, "game_state_discard_hand_to_lane_discard", gs_id, lane_selected, hand_card_offset);

            to_exec.to_exec = command::DEFENDER_DISCARD_TO;

            update = true;
        }

        if(ImGui::Button("Pass"))
        {
            to_exec.to_exec = command::PASS;
            to_exec.player = game_state::DEFENDER;
            update = true;
        }

        ImGui::End();
    }

    if(update)
    {
        //std::vector<game_state*> hack{&basic_state};

        //card_updater.do_update_strategy(1.f, 0.0, hack, net_state, 0);

        //to_exec.execute(sd, gs_id);

        commands.add(to_exec);
    }
}

///dear future james:
///replace the entire game state class with just an int
///delete all logic functions
///replace all interactions with javascript interactions
///replace api between rendering functions and game state with js calls
///also remember to fix mainfunc
void init_js_interop(stack_duk& sd)
{
    sd.ctx = js_interop_startup();

    std::string binmat_js = read_file("binmat_sim.js");

    register_function(sd, binmat_js, "mainfunc");

    call_global_function(sd, "mainfunc");

    sd.save_function_call_point();
}

void do_seamless_ui(stack_duk& sd, arg_idx gs_id, command_manager& commands, game_state::player_t player, game_state& basic_state, vec2f mpos, seamless_ui_state& ui_state)
{
    if(ImGui::IsMouseClicked(1))
    {
        ui_state = seamless_ui_state();
    }

    sf::Keyboard key;

    ImGuiIO& io{ImGui::GetIO()};

    io.KeysDown[sf::Keyboard::F] = key.isKeyPressed(sf::Keyboard::F);

    if(!basic_state.can_act(player, sd, gs_id))
        return;

    for(int i=0; i < NUM_LANES; i++)
    {
        card_list& cards = basic_state.get_cards(piles::LANE_DECK, i);

        if(cards.is_within(mpos))
        {
            tooltip::add("Draw to Hand");

            if(ImGui::IsMouseDoubleClicked(0))
            {
                command to_exec;

                if(player == game_state::ATTACKER)
                    to_exec.to_exec = command::ATTACK_DRAW_LANE;
                if(player == game_state::DEFENDER)
                    to_exec.to_exec = command::DEFENDER_DRAW_LANE;

                to_exec.pile = piles::LANE_DECK;
                to_exec.player = player;
                to_exec.lane_selected = i;


                commands.add(to_exec);
            }
        }

        std::vector<piles::piles_t> pile_list = {piles::DEFENDER_STACK, piles::ATTACKER_STACK, piles::LANE_DISCARD};
        std::vector<game_state::player_t> valid_players = {game_state::DEFENDER, game_state::ATTACKER, game_state::DEFENDER};
        std::vector<std::string> strings = {"Click to Select Defender Stack", "Click to Select Attacker Stack", "Click to Select Discard Pile"};

        bool any_true = false;
        std::string extra_string = "Middle Mouse to Initiate Combat";

        for(int kk=0; kk < pile_list.size(); kk++)
        {
            card_list& cx = basic_state.get_cards(pile_list[kk], i);

            if(cx.is_within(mpos) && player == valid_players[kk])
            {
                tooltip::add(strings[kk] + " " + std::to_string(i));

                if(ImGui::IsMouseClicked(0))
                {
                    ui_state.pile = pile_list[kk];
                    ui_state.lane = i;
                    ui_state.pile_selected = true;
                }

                if(player == game_state::ATTACKER)
                {
                    any_true = true;

                    if(ImGui::IsMouseClicked(2))
                    {
                        ui_state.lane = i;
                        ui_state.initiate_combat = true;
                    }
                }
            }
        }

        if(any_true)
        {
            tooltip::add(extra_string);
        }
    }

    card_list& hand = basic_state.get_hand(player);

    for(int coffset = 0; coffset < hand.cards.size(); coffset++)
    {
        card& c = hand.cards[coffset];

        if(c.is_within(mpos))
        {
            tooltip::add("Click to Select Card");

            if(ImGui::IsMouseClicked(0))
            {
                ui_state.card_selected = true;
                ui_state.selected_card_id = coffset;
            }
        }
    }

    if(ui_state.pile_selected)
    {
        tooltip::add("Pile Selected");
    }

    if(ui_state.card_selected)
    {
        tooltip::add("Card Selected");
    }


    if(ui_state.pile_selected || ui_state.card_selected)
    {
        if(ui_state.is_faceup)
            tooltip::add("Playing Face UP, F to toggle");
        else
            tooltip::add("Playing Face DOWN, F to toggle");
    }

    if(ImGui::IsKeyPressed(sf::Keyboard::F))
    {
        ui_state.is_faceup = !ui_state.is_faceup;
    }

    if((ui_state.pile_selected && ui_state.card_selected) || ui_state.initiate_combat)
    {
        commands.add(ui_state.get_command(player));

        ui_state = seamless_ui_state();
    }
}

///need to network initial card state and then we're golden
///oh and keepalive
int main(int argc, char* argv[])
{
    stack_duk sd;
    init_js_interop(sd);

    networking_init();
    network_state net_state;
    net_state.reliable_ordered.init_client();
    net_state.try_join = true;

    update_strategy card_updater;

    tests();

    sf::RenderWindow window;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    window.create(sf::VideoMode(1500, 900),"Wowee", sf::Style::Default, settings);
    window.setVerticalSyncEnabled(false);

    ImGui::SFML::Init(window);

    ImGui::NewFrame();

    ImGuiStyle& style = ImGui::GetStyle();

    style.FrameRounding = 2;
    style.WindowRounding = 2;
    style.ChildWindowRounding = 2;

    sf::Clock ui_clock;

    sf::Clock time;

    arg_idx gs_id = call_function_from_absolute(sd, "game_state_make");
    arg_idx cm_id = call_function_from_absolute(sd, "card_manager_make");

    printf("gs cm %i %i\n", gs_id.val, cm_id.val);

    call_function_from_absolute(sd, "game_state_generate_new_game", gs_id, cm_id);
    ///does not return
    ///so we can get it off the stack
    sd.pop_n(1);

    call_function_from_absolute(sd, "debug", gs_id);

    /*card_manager all_cards;
    all_cards.generate_cards();

    game_state current_game;
    current_game.generate_new_game(all_cards);*/

    game_state::player_t current_player = game_state::SPECTATOR;

    net_state.register_keepalive();

    game_state basic_state;
    basic_state.explicit_register();

    command_manager commands;
    commands.explicit_register();

    seamless_ui_state ui_state;

    while(window.isOpen())
    {
        sf::Event event;

        while(window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if(event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::Resized)
            {
                int x = event.size.width;
                int y = event.size.height;

                window.create(sf::VideoMode(x, y), "Wowee", sf::Style::Default, settings);
            }
        }

        sf::Mouse mouse;
        auto mpos = mouse.getPosition(window);

        do_ui(sd, gs_id, commands, current_player, basic_state);

        do_seamless_ui(sd, gs_id, commands, current_player, basic_state, {mpos.x, mpos.y}, ui_state);

        commands.network(net_state);
        commands.exec_all(sd, gs_id, ui_state);

        basic_state.import_state_from_js(sd, gs_id);

        ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            if(current_player != game_state::DEFENDER && ImGui::Button("Attacker"))
            {
                basic_state.set_viewer_state(game_state::ATTACKER);
            }

            if(current_player != game_state::ATTACKER && ImGui::Button("Defender"))
            {
                basic_state.set_viewer_state(game_state::DEFENDER);
            }

            if(current_player == game_state::OVERLORD && ImGui::Button("See Everything"))
            {
                basic_state.set_viewer_state(game_state::OVERLORD);
            }

            if(ImGui::Button("Real State"))
            {
                basic_state.set_viewer_state(game_state::REAL_STATE);
            }

            if(current_player == game_state::SPECTATOR)
            {
                if(ImGui::Button("Lock Into Defender"))
                {
                    current_player = game_state::DEFENDER;
                }

                if(ImGui::Button("Lock Into Attacker"))
                {
                    current_player = game_state::ATTACKER;
                }
            }

        ImGui::End();

        basic_state.render(window, basic_state.viewer);

        card_list visible_to = basic_state.get_all_visible_cards(basic_state.viewer);

        std::deque<std::string> reversed;

        if(visible_to.cards.size() > 0)
        {
            for(card& c : visible_to.cards)
            {
                if(c.is_within({mpos.x, mpos.y}))
                {
                    reversed.push_back(c.get_string());
                }
            }
        }

        if(reversed.size() > 0)
        {
            //reversed.push_front("BOTTOM");
            reversed.push_back("Stack Top");
        }

        std::reverse(reversed.begin(), reversed.end());

        for(auto& i : reversed)
        {
            tooltip::add(i);
        }

        ///BETTER INTERACTION
        {

        }

        double diff_s = time.restart().asMicroseconds() / 1000. / 1000.;

        net_state.tick(diff_s);

        ///if turn taken, do network updater

        if(net_state.connected())
        {
            for(network_data& i : net_state.available_data)
            {
                if(i.processed)
                    continue;

                int32_t internal_counter = i.data.internal_counter;

                int32_t send_mode = 0;

                i.data.handle_serialise(send_mode, false);

                serialise_data_helper::send_mode = send_mode;

                if(send_mode != 1)
                {
                    serialise_host_type host_id;

                    i.data.handle_serialise(host_id, false);

                    serialise_data_helper::ref_mode = 0;

                    serialisable* found_s = net_state.get_serialisable(host_id, i.object.serialise_id);

                    if(found_s == nullptr)
                    {
                        i.set_complete();
                        continue;
                    }

                    i.data.force_serialise(found_s, false);
                }

                if(send_mode == 1)
                {
                    serialisable::reset_network_state();

                    serialise_data_helper::ref_mode = 1;

                    std::cout << "got full gamestate" << std::endl;

                    serialise ser = i.data;

                    net_state.register_keepalive();
                    basic_state.explicit_register();
                    commands.explicit_register();

                    ser.handle_serialise(basic_state, false);
                    ser.handle_serialise_no_clear(commands, false);

                    basic_state.propagate_lane_decks_to_js(sd, gs_id);
                }

                i.set_complete();

                //handle_unprocessed();
            }
        }

        ImGui::Begin("Networking");

        if(ImGui::Button("Massive") && net_state.connected())
        {
            serialisable::reset_network_state();

            serialise_data_helper::send_mode = 1;
            serialise_data_helper::ref_mode = 1;

            net_state.register_keepalive();
            basic_state.explicit_register();
            commands.explicit_register();

            serialise ser;
            ser.default_owner = net_state.my_id;

            ser.handle_serialise(serialise_data_helper::send_mode, true);

            ser.handle_serialise(basic_state, true);
            ser.handle_serialise_no_clear(commands, true);

            network_object no_test;
            no_test.serialise_id = -2;

            net_state.forward_data(no_test, ser);
        }

        ImGui::End();

        if(tooltip::current.size() > 0)
            ImGui::SetTooltip(tooltip::current.c_str());

        tooltip::current.clear();

        ImGui::Render();

        window.display();

        window.clear();

        ImGui::SFML::Update(ui_clock.restart());
    }

    return 0;
}
