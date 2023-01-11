// sample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

using namespace std;                                                            

#include "gist.h"
#include "gist_extensions.h"
#include "gist_rtpred_point.h"

int query(gist& sr_index)
{
	double c[]={20.0, 20.0};
	rt_point *p=new rt_point(2, c); 
	rt_query_t q(rt_query_t::rt_contains, rt_query_t::rt_pointarg, p); 

	// create a cursor (or iterator) based on the specified query
	gist_cursor_t cursor;

	// Max 1
	int k=1;

	if(sr_index.fetch_init(cursor, &q, k)!=RCOK)
	{
		cerr << "Can't initialize cursor." << endl;
		return(eERROR);
	}

	// now do the lookup itself
	
	bool eof=false;

	// found coordinates & corresponding labels will be put in these variables
	char label[gist_p::max_tup_sz];
	char coord[gist_p::max_tup_sz];

	// used to specify the size of coordinates & labels
	smsize_t coord_len, label_len;

	if(!eof) 
	{
		coord_len=gist_p::max_tup_sz;
		label_len=gist_p::max_tup_sz;

		// get Nth nearest coordinate & its label until done (eof)
		if(sr_index.fetch(cursor, (void *) coord, coord_len, 
			(void *) label, label_len, eof)!=RCOK)
		{
			cerr << "Can't fetch from cursor." << endl;
		}
		if(!eof) 
		{
			// process key and data..
			double ll_x, ll_y, ur_x, ur_y;
			int i;

			// cast char data back to coordinates & labels
			ll_x=*((double *) coord);
			ur_x=*(((double *) coord)+1);
			ll_y=*(((double *) coord)+2);
			ur_y=*(((double *) coord)+3);
			i=*((int *) label);

		}
	}

	return 0;
}

int main()
{
	gist sr_index;
	
	int nSize = sizeof(sr_index);

	struct _finddata_t file;
	long nFile;
	nFile = _findfirst("rtree-file", &file);
	if (-1L != nFile)
	{
		if (0 != remove(file.name))
		{
//			throw PYXException(kstrCantDeleteFile, file.name);
		}
	}

	// create a new index file, using the given extension 
	// (here an rtree, specified by &rt_rect_ext)
	int nRC = sr_index.create("rtree-file", &rt_rect_ext);

	// two coordinates & matching labels

	// insert above two coordinates & labels in the rtree
	for(int i = 0; i<200; i++)
	{
	
		double rect[]={i, i+5, i, i+5};
		int label=i;
		sr_index.insert((void *) rect, sizeof(rect), (void *) &label, sizeof(label));
	}

	// flush the tree
	sr_index.flush();

	for(int j=0; j<1; j++)
	{
		query(sr_index);
	}


	// That's it!
	cout << "Done.\n";
	sr_index.close();	
	return 0;
}
