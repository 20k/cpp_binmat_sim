#include <iostream>
#include <SFML/Graphics.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui-SFML.h>
#include <imgui/imgui_internal.h>
#include <vec/vec.hpp>
#include "../serialise/serialise.hpp"
#include "manager.hpp"
#include <math.h>

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
    std::vector<card*> cards;

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
};

struct lane
{
    card_list lane_discard;
    card_list defender_stack;
    card_list attacker_stack;
};

struct board
{
    std::array<card_list, 6> lanes;

    card_list attacker_deck;
    card_list attacker_discard;
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

        double diff_s = time.restart().asMicroseconds() / 1000. / 1000.;

        ImGui::Render();

        window.display();

        window.clear();

        ImGui::SFML::Update(ui_clock.restart());
    }

    return 0;
}
