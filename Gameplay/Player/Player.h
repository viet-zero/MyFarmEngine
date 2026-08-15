class Player
{
public:

    void Update(float dt);

    void Move();

    void Interact();

    void UseTool();

    void OpenInventory();

private:

    Inventory inventory;

};