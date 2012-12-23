#PRINTS 5, 2, 3, 5, 0#
PROGRAM
INT x, a;
BEGIN
	x = 5;
	PRINTF x;

	a = 3;

	if [x > 1]
	BEGIN
		INT x;
		x = 2;
		PRINTF x;

		PRINTF a;
		a = 0;
	END

	PRINTF x;
	PRINTF a;
END.
