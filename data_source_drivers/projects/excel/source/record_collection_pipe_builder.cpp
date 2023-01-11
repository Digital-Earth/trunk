/******************************************************************************
record_collection_pipe_builder.cpp

begin      : 08/07/2013
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define EXCEL_SOURCE

#include "record_collection_pipe_builder.h"
#include "excel_record_collection_process.h"
#include "csv_record_collection_process.h"
#include "exceptions.h"

#include "pyxis/utility/excel.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/utility/file_utils.h"

#include <boost/algorithm/string.hpp>

// {EE299A92-143C-4EEF-8B88-D888ED8F13F3}
PYXCOM_DEFINE_CLSID(RecordCollectionPipeBuilder, 
0xee299a92, 0x143c, 0x4eef, 0x8b, 0x88, 0xd8, 0x88, 0xed, 0x8f, 0x13, 0xf3);

PYXCOM_CLASS_INTERFACES(RecordCollectionPipeBuilder, IPipeBuilder::iid, PYXCOM_IUnknown::iid);

RecordCollectionPipeBuilder::RecordCollectionPipeBuilder()
{
	m_vecSupportedExtensions.push_back("csv");
	// m_vecSupportedExtensions.push_back("xls"); Not currently supported.
	// m_vecSupportedExtensions.push_back("xlsx"); Not currently supported.
}

void RecordCollectionPipeBuilder::createCsvPipeline(std::vector<boost::intrusive_ptr<IProcess> > & result,const boost::filesystem::path &path) const
{
	try 
	{
		boost::intrusive_ptr<IProcess> pathProcess = PYXCOMCreateInstance<IProcess,PathProcess>();
		std::map<std::string, std::string> mapAttr;
		mapAttr["uri"] = FileUtils::pathToString(path);
		pathProcess->setAttributes(mapAttr);

		boost::intrusive_ptr<IProcess> csvProcess = PYXCOMCreateInstance<IProcess,CsvRecordCollectionProcess>();

		csvProcess->getParameter(0)->addValue(pathProcess);
		csvProcess->setProcName(FileUtils::pathToString(path.leaf()));
		csvProcess->setProcDescription("CSV table");

		if (csvProcess->initProc(true) == IProcess::knInitialized)
		{
			result.push_back(csvProcess);
		}
	}
	catch (PYXException&)
	{
		TRACE_INFO("Unable to build a pipeline for: " << FileUtils::pathToString(path));
	}	
}

void RecordCollectionPipeBuilder::createExcelPipelines(std::vector<boost::intrusive_ptr<IProcess> > & result,const boost::filesystem::path &path) const
{
	try 
	{
		auto workbookName = FileUtils::pathToString(path.leaf());

		auto implementation = Excel::IExcel::Implementation();
		auto workbook = implementation->CreateWorkbook(FileUtils::pathToString(path));

		std::vector<std::string> tableNames;
		
		workbook->GetTableNames(tableNames);

		boost::intrusive_ptr<IProcess> pathProcess = PYXCOMCreateInstance<IProcess,PathProcess>();
		std::map<std::string, std::string> mapAttr;
		mapAttr["uri"] = FileUtils::pathToString(path);
		pathProcess->setAttributes(mapAttr);

		for(auto & tableName : tableNames)
		{
			auto table = workbook->CreateTable(tableName);

			if (table->IsEmpty())
			{
				continue;
			}

			boost::intrusive_ptr<IProcess> excelProcess = PYXCOMCreateInstance<IProcess,ExcelRecordCollectionProcess>();

			excelProcess->getParameter(0)->addValue(pathProcess);

			std::map<std::string, std::string> excelMapAttr;
			excelMapAttr["Table"] = tableName;
			excelProcess->setAttributes(excelMapAttr);
			
			excelProcess->setProcName(tableName + " : " + workbookName);
			excelProcess->setProcDescription("Excel table");


			if (excelProcess->initProc(true) == IProcess::knInitialized)
			{
				result.push_back(excelProcess);
			}
		}

	}
	catch (PYXException&)
	{
		TRACE_INFO("Unable to build a pipeline for: " << FileUtils::pathToString(path));
	}	
}

PYXPointer<IProcess> RecordCollectionPipeBuilder::buildPipeline(PYXPointer<PYXDataSet> pDataSet) const
{
	std::vector< PYXPointer<IProcess> > vecPipelines;
	if (isDataSourceSupported(pDataSet->getUri(), eCheckOptions::knLenient))
	{
		auto path = FileUtils::stringToPath(pDataSet->getUri());
		auto extension = FileUtils::getExtensionNoDot(path);

		if (boost::iequals(extension, "csv"))
		{
			createCsvPipeline(vecPipelines, path);
		}
		else
		{
			createExcelPipelines(vecPipelines, path);
		}
	}

	if (!vecPipelines.empty())
	{
		return vecPipelines[0];
	}

	return nullptr;
}
