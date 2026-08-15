class World
{
public:

    void Load();

    void Update(float dt);

    void Render();

private:

    Farm farm;

    Town town;

    Mine mine;

    Forest forest;

};