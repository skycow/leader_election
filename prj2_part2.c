active:
d := indent;
loop
  /* start phase */
  send(d);
  receive(e);
  if e = ident then announce elected;
  end if
  if d > e then send(d) else send(e) end if;
  receive(f);
  if f = ident then announce elected;
  end if
  if e >= max(d,f) then d := e else goto relay end if;
end loop

relay:
loop
  receive(d);
  if d = ident then announce elected;
  end if
  send(d);
end loop

/* Dolev, Klawe & Rodeh for leader election in unidirectional ring
 * `An O(n log n) unidirectional distributed algorithm for extrema
 * finding in a circle,'  J. of Algs, Vol 3. (1982), pp. 245-260
 */

#define N	8	/* nr of processes (use 5 for demos) */
#define I	3	/* node given the smallest number    */
#define L	16	/* size of buffer  (>= 2*N) */

mtype = { one, two, winner };
chan q[N] = [L] of { mtype, byte};

byte nr_leaders = 0;

proctype node (chan inp, out; byte mynumber)
{	
  bit Active = 1, know_winner = 0;
	byte nr, maximum = mynumber, neighbourR;

	xr inp;
	xs out;

	printf("MSC: %d\n", mynumber);
	out!one(mynumber);
end:	do
	:: inp?one(nr) ->
		if
		:: Active -> 
			if
			:: nr != maximum ->
				out!two(nr);
				neighbourR = nr
			:: else ->
				/* Raynal p.39:  max is greatest number */
				assert(nr == N);
				know_winner = 1;
				out!winner,nr;
			fi
		:: else ->
			out!one(nr)
		fi

	:: inp?two(nr) ->
		if
		:: Active -> 
			if
			:: neighbourR > nr && neighbourR > maximum ->
				maximum = neighbourR;
				out!one(neighbourR)
			:: else ->
				Active = 0
			fi
		:: else ->
			out!two(nr)
		fi
	:: inp?winner,nr ->
		if
		:: nr != mynumber ->
			printf("MSC: LOST\n");
		:: else ->
			printf("MSC: LEADER\n");
			nr_leaders++;
			assert(nr_leaders == 1)
		fi;
		if
		:: know_winner
		:: else -> out!winner,nr
		fi;
		break
	od
}

init {
	byte proc;
	atomic {
		proc = 1;
		do
		:: proc <= N ->
			run node (q[proc-1], q[proc%N], (N+I-proc)%N+1);
			proc++
		:: proc > N ->
			break
		od
	}
}


