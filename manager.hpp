#ifndef MANAGER_HPP_INCLUDED
#define MANAGER_HPP_INCLUDED

struct asteroid;
struct player_ship;
struct ship;
struct torpedo;

template<typename T>
struct manager : serialisable
{
    bool cleanup = false;

    std::vector<T*> elems;

    T* make_new()
    {
        T* n = new T();
        n->make_dirty();

        elems.push_back(n);

        return n;
    }

    template<typename U>
    U* make_new_of()
    {
        U* n = new U();
        n->make_dirty();

        elems.push_back(n);

        return n;
    }

    void force_clear_all()
    {
        for(auto& i : elems)
        {
            delete i;
        }

        elems.clear();
    }

    template<typename manager_type = manager<T>>
    void tick(double diff_s)
    {
        for(int i=0; i < (int)elems.size(); i++)
        {
            //if(!elems[i]->owned_by_host)
            //    continue;

            elems[i]->tick(diff_s);
        }

        for(int i=0; i < (int)elems.size(); i++)
        {
            if(!elems[i]->collides)
                continue;

            vec2f p1 = elems[i]->info.pos;
            float r1 = elems[i]->approx_rad;

            for(int j = 0; j < (int)elems.size(); j++)
            {
                if(elems[j]->collides)
                {
                    if(j <= i)
                        continue;
                }

                vec2f p2 = elems[j]->info.pos;
                float r2 = elems[j]->approx_rad;

                if((p2 - p1).squared_length() > ((r1 * 1.5f + r2 * 1.5f) * (r1 * 1.5f + r2 * 1.5f)) * 4.f)
                    continue;

                if(elems[i]->collides_with(*elems[j]))
                {
                    elems[i]->on_collide(*dynamic_cast<manager_type*>(this), *elems[j]);
                    elems[j]->on_collide(*dynamic_cast<manager_type*>(this), *elems[i]);
                }
            }
        }
    }

    void clean()
    {
        for(int i=0; i < (int)elems.size(); i++)
        {
            if(elems[i]->should_cleanup())
            {
                elems[i]->on_cleanup();
            }

            ///not an error, means that for the moment, on cleanup can terminate cleanup
            ///hacky but :shrug:
            if(elems[i]->should_cleanup())
            {
                T* e = elems[i];

                elems.erase(elems.begin() + i);
                i--;
                delete e;
                continue;
            }
        }
    }

    void render(sf::RenderWindow& window)
    {
        for(int i=0; i < (int)elems.size(); i++)
        {
            elems[i]->render(window);
        }
    }

    virtual void do_serialise(serialise& s, bool ser) override
    {
        /*if(serialise_data_helper::send_mode == 0)
        {
             s.handle_serialise(elems, ser);
        }*/

        if(serialise_data_helper::send_mode == 1)
        {
            s.handle_serialise(elems, ser);
        }

        if(serialise_data_helper::send_mode == 2)
        {
            if(ser)
            {
                int32_t extra = 0;

                for(T* o : elems)
                {
                    if(o->dirty || o->requires_attention)
                    {
                        extra++;
                    }
                }

                s.handle_serialise(extra, ser);

                for(T* o : elems)
                {
                    if(o->dirty || o->requires_attention)
                    {
                        s.handle_serialise(o, ser);
                    }
                }
            }
            else
            {
                int32_t extra = 0;

                s.handle_serialise(extra, ser);

                for(int i=0; i<extra; i++)
                {
                    T* o = nullptr;

                    s.handle_serialise(o, ser);

                    if(!o->owned_by_host)
                    {
                        elems.push_back(o);
                    }
                }
            }
        }

        for(T* e : elems)
        {
            e->handled_by_client = true;
        }

        handled_by_client = true;
    }
};

struct trans
{
    vec2f pos;
    float rotation = 0;
    float scale = 1;
    int flip = 0;

    vec2f from_world(vec2f in_pos)
    {
        vec2f local = ((in_pos - pos) / scale).rot(-rotation);

        if(flip)
        {
            local.x() = -local.x();
        }

        return local;
    }

    vec2f to_world(vec2f in_pos)
    {
        if(flip)
        {
            in_pos.x() = -in_pos.x();
        }

        return in_pos.rot(rotation) * scale + pos;
    }
};


struct basic_entity : serialisable
{

};

#endif // MANAGER_HPP_INCLUDED
