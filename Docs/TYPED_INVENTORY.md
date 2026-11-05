# P2-002A — Typed inventory core

## P2-002B update — 2026-11-05

[New farming/storage contract](FARMING_HOME_STORAGE.md): one bag (default 12 slots) plus distinct HomeStorage (48), same Inventory algorithm. ItemIds 1006/1007 seed.carrot/crop.carrot and 1008/1009 seed.potato/crop.potato added, maxStack20. Watering now works with owned can. Home crop/apple harvest goes to warehouse; ground pickup/outside harvest remains bag. Counts stay model-derived; K panel read-only, no auto-withdraw or SQL save. Earlier P2-002A descriptions below are historical (its seven-item table, permanent hotbar and unimplemented watering).

2026-11-05. Logic-first slice of P2-002; keeps existing engine and assets. Does not require eight-direction/outfit artwork.

## Single owner and data

InteractionActor owns MyGame::Inventory from the previously empty Gameplay/Inventory files. The old writable inventoryCount/inventoryCapacity are removed. SceneSnapshot.inventoryCount is only a derived ResourceCount diagnostic for existing scene tests, excluding tools; it cannot spend or grant items. Bag() exposes const slots/counts for inspection. There is no second inventory.

Fixed numeric IDs and stable keys:

| ID | Key | Stack limit |
| --- | --- | --- |
| 1001 | seed.turnip | 20 |
| 1002 | crop.turnip | 20 |
| 1003 | fruit.apple | 20 |
| 1004 | food.cake | 10 |
| 1005 | resource.wood | 20 |
| 2001 | tool.watering_can | 1 |
| 2002 | tool.axe | 1 |

Definitions are currently a compiled table in Inventory.cpp; Items.json remains an empty legacy stub, not a working item loader. These IDs are intended as future save/database keys, but no save schema is implemented yet. Wood/cake definitions do not mean tree drops/eating exist.

Bag has 12 slots; each slot contains one item type. Starter owns watering can x1, axe x1 and turnip seeds x3: 3 occupied slots, total quantity 5, resource quantity 3. Test fixtures may choose 0..12 slots. Counts cannot be assigned directly. No item cloning or unknown ID insertion through public add/remove APIs.

CanAdd counts free space in matching stacks AND empty slots, so an apparently occupied bag can still accept matching resources. TryAdd tops up existing stacks then takes empty slots. Add/remove are all-or-nothing, reject nonpositive/unknown/excessive quantities and normalize empty stacks. Slot access is const. This is single-thread gameplay state, not a thread-safe/shared server inventory.

## Impact transaction

Plant requires seed.turnip in the selected hotbar binding AND at least one seed in bag. Apples/turnips cannot stand in for seeds. At IMPACT, exactly one seed is removed. Crop harvest grants crop.turnip, orchard grants fruit.apple; pickup grants its declared item (demo defaults apple). Axe use requires tool.axe in the bag.

Start records selected binding and target reward ID; changing those while pending rejects impact. The active action keeps the slot captured at Start even if the player highlights another slot; rebinding that original slot is rejected. Existing range/facing/LOS/stamina/cooldown/target/action token checks remain. Resource availability and capacity are checked again at impact.

After event queue capacity check, bag changes are prepared on a value copy; failed add/remove does not charge stamina/change target/publish FX. On success the candidate replaces the bag and the existing target/events transaction completes. Existing exact-once marker protection remains. This guarantees expected gameplay failures are atomic; it is not a database transaction or exception/crash durability guarantee.

## Hotbar and visuals

Keys 1..4 bind ItemIds (watering can, axe, turnip seed, cake), not atlas frame numbers. Tool checks follow ItemId, so moving a binding does not change item semantics. Current gameplay has no drag/rebind UI; rebind behavior is tested via model API. Counts are drawn from the same bag using small numeric strokes; no new image/font assets. Existing four item images remain temporary: seed slot currently reuses old fruit-looking icon, and cake/watering actions are not implemented. An icon is not evidence of a working mechanic.

No bag grid, sorting/split-stack/move UI, item tooltip, equipment swapping UI, money, watering, three-crop loop, selling, shop or persistence in this slice. Default cap changed from a counter limit of 12 resource units to 12 real slots with per-type stacks.

## Verification

New gameplay.inventory tests cover stack split/merge/free-slot, full bag matching stack, all-or-nothing add/remove, invalid/huge amounts/unknown IDs, unstackable tools, starter kit, apples-not-seeds, ItemId-based bindings, tool loss at impact, reward/binding changed while pending and typed pickup/harvest. Existing interaction/action tests use real bags; failed impact compares the whole bag. Live scene test checks typed starter, apple impact and seed/turnip separation at crop harvest. Full suite also covers animation, fixed 30/60/144 cadence, geometry, runtime and executable graphics smoke.

No manual extended gameplay session or numeric UI pixel approval is claimed; final build/test results are recorded in PROGRESS.md. Original assets and runtime artwork remain unchanged.

## SQL Server local preflight

User authorized using installed SQL software when persistence is needed. Read-only check on 2026-11-05 found MSSQL$SQLEXPRESS running, SQLBrowser stopped, 64-bit ODBC Driver 17/18 installed. A real System.Data.SqlClient Windows Authentication connection to .\SQLEXPRESS/master succeeded: Express Edition (64-bit), ProductVersion 17.0.1000.7. Only SELECT SERVERPROPERTY(Edition/ProductVersion) executed; no database/table/user created or service/network configuration changed.

The local probe used encrypted connection with TrustServerCertificate for this development instance; this does not certify production TLS. It verifies local engine connectivity through .NET only, not the C++ ODBC backend or game save/load. SQL I/O is not added to update/render. A later persistence pilot must test typed snapshots, prepared statements, transactions/rollback, restart roundtrip and connection failure using a dedicated game database. No SQL password belongs in the game repository. Future public online architecture remains game -> backend/server -> SQL Server, not public database credentials embedded in client.
