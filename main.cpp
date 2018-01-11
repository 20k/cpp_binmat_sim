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

    void render_face(sf::RenderWindow& win, bool should_render_face_up)
    {
        vec3f face_up_col = {1,1,1};
        vec3f face_down_col = {0.5f, 0.5f, 0.5f};

        sf::RectangleShape shape;

        shape.setPosition(info.pos.x(), info.pos.y());

        vec2f dim = {CARD_WIDTH, 2*CARD_WIDTH};

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
        render_face(win, false);
    }

    void render_face_up(sf::RenderWindow& win)
    {
        render_face(win, true);

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
            for(int card = 0; card < CARDS_IN_LANE; card++)
            {
                card_list& cards = get_cards(piles::LANE_DECK, lane);

                cards.cards.push_back(all_cards.fetch_without_replacement());
            }
        }

        /*for(int lane = 0; lane < NUM_LANES; lane++)
        {
            card_list& cards = get_cards(piles::LANE_DECK, lane);

            printf("%i\n", cards.cards.size());
        }*/
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

    void render(sf::RenderWindow& win, player_t player)
    {
        vec2f dim = {win.getSize().x, win.getSize().y};

        vec2f centre = dim/2.f;

        float card_separation = CARD_WIDTH * 1.4f;

        for(int i=0; i < NUM_LANES; i++)
        {
            card_list current_deck = get_cards(piles::LANE_DECK, i);

            float lane_x = (card_separation * i) - (card_separation * NUM_LANES/2.f) + centre.x();

            for(card* c : current_deck.cards)
            {
                c->info.pos = {lane_x, centre.y()};

                if(is_visible(c, piles::LANE_DECK, player, i))
                {
                    c->render_face_up(win);
                }
                else
                {
                    c->render_face_down(win);
                }
            }
        }
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

        current_game.render(window, game_state::OVERLORD);

        double diff_s = time.restart().asMicroseconds() / 1000. / 1000.;

        ImGui::Render();

        window.display();

        window.clear();

        ImGui::SFML::Update(ui_clock.restart());
    }

    return 0;
}
