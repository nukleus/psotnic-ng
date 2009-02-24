#ifndef REPEAT_H
#define REPEAT_H

class repeat : public CustomDataObject
{
    public:
    pstring<> str;
    time_t when;
    time_t creation;
    
    bool hit(const char *s);

    repeat();
    ~repeat();
};

#endif /* REPEAT_H */
