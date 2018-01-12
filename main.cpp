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

#define CARD_WIDTH 25
#define CARD_HEIGHT CARD_WIDTH * 1.7

struct card : basic_entity
{
    bool face_down = false;
    bool hidden = false;

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

    bool is_normal()
    {
        return !is_modifier();
    }

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
                card* c = make_new();

                c->set_card_from_offsets(suit_offset, card_offset);
            }
        }
    }

    int card_fetch_counter = 0;

    card* fetch_without_replacement()
    {
        return elems[card_fetch_counter++];
    }

    void reset_fetching()
    {
        card_fetch_counter = 0;
    }
};

void shuffle_cards(std::vector<card*>& cards)
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

struct card_list
{
    ///left is bottom, right is top
    std::vector<card*> cards;

    bool can_be_drawn_from = false;
    bool hidden_to_opposition_if_face_down = true;
    bool hidden_to_opposition_if_face_up = true;

    bool face_up = false;

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

    bool add_face_down_card(card* c)
    {
        if(cards.size() == 0)
        {
            face_up = false;
        }

        if(is_face_up())
            return false;

        cards.push_back(c);
    }

    void add_face_up_card(card* c)
    {
        face_up = true;

        cards.push_back(c);
    }

    int calculate_stack_damage()
    {
        int sum = 0;

        for(card* c : cards)
        {
            if(c->is_modifier())
                continue;

            sum += c->get_value();
        }

        int powers_to_bump_up = 0;

        for(card* c : cards)
        {
            if(c->is_type(card::WILD))
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

        return sum;
    }

    void prune_to_face_up()
    {
        for(int i=0; i < cards.size(); i++)
        {
            if(!cards[i]->is_face_up())
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

    bool take_top_card(card_list& other)
    {
        if(other.cards.size() == 0)
        {
            return false;
        }

        card* c = other.cards.back();

        other.cards.pop_back();

        cards.push_back(c);

        return true;
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

    bool remove_card(card* c)
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
        for(card* c : cards)
        {
            if(c->is_type(type))
                return true;
        }

        return false;
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
}

struct lane
{
    ///lane discard, lane deck, defender stack, attacker stack
    std::array<card_list, 4> card_piles;
};

struct game_state
{
    std::array<lane, 6> lanes;

    ///Attacker deck, attacker discard, attacker hand, defender hand
    std::array<card_list, 4> piles;

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

            for(card* c : cards.cards)
            {
                if(c->is_face_up())
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

    bool lane_has_faceup_top_card(int lane)
    {
        return lane >= 3;
    }

    #define NUM_LANES 6

    void generate_new_game(card_manager& all_cards)
    {
        all_cards.reset_fetching();

        #define CARDS_IN_LANE 13

        shuffle_cards(all_cards.elems);

        for(int i=0; i < piles::COUNT; i++)
        {
            if(i == piles::LANE_DECK || i == piles::LANE_DISCARD || i == piles::ATTACKER_STACK || i == piles::DEFENDER_STACK)
            {
                for(int lane = 0; lane < NUM_LANES; lane++)
                {
                    card_list& clist = get_cards((piles::piles_t)i, lane);

                    clist = card_list();
                }
            }
            else
            {
                card_list& clist = get_cards((piles::piles_t)i, -1);

                clist = card_list();
            }
        }

        for(int lane=0; lane < NUM_LANES; lane++)
        {
            card_list& cards = get_cards(piles::LANE_DECK, lane);

            for(int card = 0; card < CARDS_IN_LANE; card++)
            {
                cards.cards.push_back(all_cards.fetch_without_replacement());
            }

            for(card* c : cards.cards)
            {
                c->face_down = true;
            }

            if(lane_has_faceup_top_card(lane))
            {
                cards.cards.back()->face_down = false;
            }
        }
    }

    bool is_visible(card* check, piles::piles_t pile, player_t player, int lane)
    {
        card_list visible_cards = get_visible_pile_cards_as(pile, player, lane);

        for(card* c : visible_cards.cards)
        {
            if(c == check)
                return true;
        }

        return false;
    }

    void render_row(piles::piles_t pile, int cnum, int max_num, vec2f centre, player_t player, sf::RenderWindow& win, float yoffset)
    {
        card_list current_deck = get_cards(pile, cnum);

        float card_separation = CARD_WIDTH * 1.4f;

        float lane_x = (card_separation * cnum) - (card_separation * max_num/2.f) + centre.x();

        for(card* c : current_deck.cards)
        {
            c->info.pos = {lane_x, centre.y() + yoffset};

            if(is_visible(c, pile, player, cnum))
            {
                c->render_face_up(win);
            }
            else
            {
                c->render_face_down(win);
            }
        }

        vec2f br = {lane_x, centre.y() + yoffset};

        br += (vec2f){CARD_WIDTH/2.f, CARD_HEIGHT/2.f};

        if(current_deck.cards.size() > 0)
            render_font(win, std::to_string(current_deck.cards.size()), br, {1,1,1,1}, 0.6f);
    }

    void render_individual(piles::piles_t pile, int cnum, int max_num, vec2f centre, player_t player, sf::RenderWindow& win, float yoffset)
    {
        card_list current_deck = get_cards(pile, cnum);

        float card_separation = CARD_WIDTH * 1.4f;

        float lane_x = (card_separation * cnum) - (card_separation * max_num/2.f) + centre.x();

        card* c = current_deck.cards[cnum];

        c->info.pos = {lane_x, centre.y() + yoffset};

        if(is_visible(c, pile, player, cnum))
        {
            c->render_face_up(win);
        }
        else
        {
            c->render_face_down(win);
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

    card_list& get_lane_stack_for_player(player_t player, int lane)
    {
        if(player == ATTACKER)
        {
            return get_cards(piles::ATTACKER_STACK, lane);
        }

        if(player == DEFENDER)
        {
            return get_cards(piles::DEFENDER_STACK, lane);
        }

        printf("Bad player in get lane stack for player\n");

        assert(false);
    }

    bool transfer_top_card(piles::piles_t to_pile, int to_lane, piles::piles_t from_pile, int from_lane)
    {
        card_list& to_cards = get_cards(to_pile, to_lane);
        card_list& from_cards = get_cards(from_pile, from_lane);

        return to_cards.take_top_card(from_cards);
    }

    bool lane_protected(int lane)
    {
        card_list& defender_cards = get_cards(piles::DEFENDER_STACK, lane);

        return defender_cards.cards.size() > 0;
    }

    bool draw_from(piles::piles_t pile, int lane, player_t player)
    {
        if(player != ATTACKER && player != DEFENDER)
            return false;

        card_list& hand = get_hand(player);

        if(player == ATTACKER)
        {
            if(pile == piles::ATTACKER_DECK)
            {
                bool success = transfer_top_card(piles::ATTACKER_HAND, -1, piles::ATTACKER_DECK, -1);

                if(!success)
                {
                    success = get_cards(piles::ATTACKER_HAND, -1).shuffle_in(get_cards(piles::ATTACKER_DISCARD));

                    printf("no cards in attacker deck\n");

                    if(success)
                    {
                        printf("shuffled in discard\n");
                    }
                    else
                    {
                        printf("could not shuffle in discard");
                    }

                    return success;
                }

                return true;
            }

            if(pile == piles::LANE_DECK)
            {
                if(lane_protected(lane))
                    return false;

                bool success = transfer_top_card(piles::ATTACKER_HAND, -1, piles::LANE_DECK, lane);

                if(!success)
                {
                    printf("no cards in lane deck\n");

                    success = get_cards(piles::LANE_DECK, lane).shuffle_in(get_cards(piles::LANE_DISCARD, lane));

                    if(success)
                    {
                        printf("shuffled in lane discard\n");
                    }
                    else
                    {
                        printf("attacker wins\n");
                    }

                    return success;
                }

                return true;
            }
        }

        if(player == DEFENDER)
        {
            if(player == piles::LANE_DECK)
            {
                bool success = transfer_top_card(piles::DEFENDER_HAND, -1, piles::LANE_DECK, lane);

                if(!success)
                {
                    printf("no cards in lane deck\n");

                    success = get_cards(piles::LANE_DECK, lane).shuffle_in(get_cards(piles::LANE_DISCARD, lane));

                    if(success)
                    {
                        printf("shuffled in lane discard\n");
                    }
                    else
                    {
                        printf("attacker wins\n");
                    }

                    return success;
                }

                return true;
            }
        }

        return false;
    }

    bool can_play_face_up_on(player_t player, card* to_play, card_list& appropriate_stack)
    {
        ///if I play a break and the stack does not contain a break and is not empty, can be played face up
        if(to_play->is_type(card::BREAK) && !appropriate_stack.contains(card::BREAK) && appropriate_stack.cards.size() > 0)
            return true;

        ///if I'm an attacker, can play a face up bounce on an empty stack
        if(to_play->is_type(card::BOUNCE) && player == ATTACKER && appropriate_stack.cards.size() == 0)
            return true;

        return false;
    }

    bool can_play_face_down_on(player_t player, card* to_play, card_list& appropriate_stack)
    {
        if(!appropriate_stack.accepts_face_down_cards())
            return false;

        ///no 2 breaks in a stack
        if(to_play->is_type(card::BREAK) && appropriate_stack.contains(card::BREAK))
            return false;

        ///cannot ever play a break on an empty stack
        if(to_play->is_type(card::BREAK) && appropriate_stack.cards.size() == 0)
            return false;

        return true;
    }

    void trigger_combat()
    {
        printf("combat\n");
    }

    bool play_card_on_stack(player_t player, card* to_play, card_list& appropriate_stack, bool face_up)
    {
        std::cout << "fup " << face_up << std::endl;

        if(face_up && can_play_face_up_on(player, to_play, appropriate_stack))
        {
            to_play->face_down = false;

            bool is_face_down = appropriate_stack.is_face_down();

            appropriate_stack.add_face_up_card(to_play);

            ///went from face down to face up
            if(is_face_down)
            {
                trigger_combat();
            }

            return true;
        }

        if(!face_up && can_play_face_down_on(player, to_play, appropriate_stack))
        {
            to_play->face_down = true;

            appropriate_stack.add_face_down_card(to_play);

            return true;
        }

        return false;
    }

    bool play_to_stack_from_hand(player_t player, int lane, int card_offset, bool face_up)
    {
        card_list& hand = get_hand(player);

        if(card_offset < 0 || card_offset >= hand.cards.size())
            return false;

        card* to_play = hand.cards[card_offset];

        ///if false, tried to play a card not in hand
        ///can obviously never happen in proper play
        //if(!hand.remove_card(to_play))
        //    assert(false);

        card_list& appropriate_stack = get_lane_stack_for_player(player, lane);

        bool success = play_card_on_stack(player, to_play, appropriate_stack, face_up);

        if(!success)
        {
            printf("nope cannot play\n");
        }

        if(success)
        {
            hand.remove_card(to_play);
        }

        return success;
    }

    player_t viewer = REAL_STATE;

    void set_viewer_state(player_t player)
    {
        viewer = player;
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

void do_ui(game_state& current_game)
{
    ImGui::Begin("Actions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

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

    static int lane_selected = 0;
    static int is_faceup = false;
    static int hand_card_offset = 0;

    ImGui::InputInt("Lane", &lane_selected);
    ImGui::InputInt("Face Up?", &is_faceup);
    ImGui::InputInt("Hand Card Number", &hand_card_offset);

    is_faceup = clamp(is_faceup, 0, 1);
    //hand_card_offset = clamp(hand_card_offset, 0, current);

    lane_selected = clamp(lane_selected, 0, 6);

    if(ImGui::Button("Attacker Draw from Lane Deck"))
    {
        current_game.draw_from(piles::LANE_DECK, lane_selected, game_state::ATTACKER);
    }

    if(ImGui::Button("Attacker Draw From Attacker Deck"))
    {
        current_game.draw_from(piles::ATTACKER_DECK, -1, game_state::ATTACKER);
    }

    if(ImGui::Button("Attacker Play to Stack"))
    {
        current_game.play_to_stack_from_hand(game_state::ATTACKER, lane_selected, hand_card_offset, is_faceup);
    }

    if(ImGui::Button("Attacker Initiate Combat At Lane"))
    {

    }

    ImGui::NewLine();

    if(ImGui::Button("Defender Draw From Lane Deck"))
    {
        current_game.draw_from(piles::LANE_DECK, lane_selected, game_state::DEFENDER);
    }

    if(ImGui::Button("Defender Play to Stack"))
    {
        current_game.play_to_stack_from_hand(game_state::DEFENDER, lane_selected, hand_card_offset, is_faceup);
    }

    if(ImGui::Button("Discard Card to Lane Discard Pile"))
    {

    }

    ImGui::End();
}

int main()
{
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

    card_manager all_cards;
    all_cards.generate_cards();

    game_state current_game;
    current_game.generate_new_game(all_cards);

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

        do_ui(current_game);

        ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            if(ImGui::Button("Attacker"))
            {
                current_game.set_viewer_state(game_state::ATTACKER);
            }

            if(ImGui::Button("Defender"))
            {
                current_game.set_viewer_state(game_state::DEFENDER);
            }

            if(ImGui::Button("See Everything"))
            {
                current_game.set_viewer_state(game_state::OVERLORD);
            }

            if(ImGui::Button("Real State"))
            {
                current_game.set_viewer_state(game_state::REAL_STATE);
            }

        ImGui::End();

        current_game.render(window, current_game.viewer);

        double diff_s = time.restart().asMicroseconds() / 1000. / 1000.;

        ImGui::Render();

        window.display();

        window.clear();

        ImGui::SFML::Update(ui_clock.restart());
    }

    return 0;
}
