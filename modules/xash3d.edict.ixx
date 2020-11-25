module;

export module xash3d.edict;
import xash3d.types;
import xash3d.consts;
import xash3d.progdefs;

export namespace xash3d
{
	typedef struct edict_s edict_t;

	constexpr auto MAX_ENT_LEAFS = 48;

	struct edict_s
	{
		qboolean		free;
		int		serialnumber;

		link_t		area;		// linked to a division node or leaf
		int		headnode;		// -1 to use normal leaf check

		int		num_leafs;
		short		leafnums[MAX_ENT_LEAFS];

		float		freetime;		// sv.time when the object was freed

		void* pvPrivateData;	// Alloced and freed by engine, used by DLLs
		entvars_t		v;		// C exported fields from progs

		// other fields from progs come immediately after
	};
}