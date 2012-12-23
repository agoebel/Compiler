#PRINTS OUT 0, 1, 0#
PROGRAM
INT X, A;
BEGIN
	X = 0;
	LOOP [X < 1]
	BEGIN
		A = 0;
		LOOP [A < 2]
		BEGIN
			PRINTF A;
			A = A + 1;
		END

		PRINTF X;
		X = X + 1;
	END
END.
