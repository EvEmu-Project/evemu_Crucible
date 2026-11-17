-- Deterministic synthetic scenario (not real mission/contract processing).
-- Simulates one character receiving ISK, partial item consumption,
-- market sale, and a contract status change. Character B must be unchanged.
START TRANSACTION;
UPDATE a01_chrCharacters SET balance=balance+25000.00 WHERE characterID=90010001;
UPDATE a01_entity SET quantity=quantity-5 WHERE itemID=140010001 AND quantity>=5;
UPDATE a01_agtOffers SET stateID=2 WHERE offerID=41010001 AND stateID=1;
UPDATE a01_mktOrders SET volRemaining=volRemaining-2 WHERE orderID=51010001 AND volRemaining>=2;
UPDATE a01_ctrContracts SET acceptorID=90010002,status=1 WHERE contractId=61010001 AND status=0;
COMMIT;
