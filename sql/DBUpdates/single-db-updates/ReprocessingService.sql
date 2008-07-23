UPDATE TL2MaterialsForTypeWithActivity SET recycle = 1;

INSERT INTO TL2MaterialsForTypeWithActivity SELECT oreTypeID, 6 AS activity, mineralTypeID, amountPerBatch, 1 AS damagePerJob, 0 AS recycle FROM invOreReprocessing;

DROP TABLE invOreReprocessing;