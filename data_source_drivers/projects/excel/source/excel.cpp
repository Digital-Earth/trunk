/******************************************************************************
excel.cpp

begin      : 07/11/2007 12:00:35 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define EXCEL_SOURCE
#include "excel.h"

// local includes
#include "excel_process.h"
#include "csv_record_collection_process.h"
#include "csv_record_collection_process_v1.h"
#include "csv_record_collection_process_v2.h"
#include "excel_record_collection_process.h"
#include "record_collection_pipe_builder.h"

// pyxlib includes
#include "pyxis/utility/mem_utils.h"

PYXCOM_BEGIN_CLASS_OBJECT_TABLE
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ExcelProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(ExcelRecordCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CsvRecordCollectionProcess),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CsvRecordCollectionProcess_v1),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(CsvRecordCollectionProcess_v2),
	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(RecordCollectionPipeBuilder)
PYXCOM_END_CLASS_OBJECT_TABLE

namespace
{

//! Static object to manage module.
struct ModuleManager
{

	ModuleManager()
	{
	}

	~ModuleManager()
	{
	}

} moduleManager;

}
