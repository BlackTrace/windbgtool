// DotNetInfoParser.cpp : Implementation of CDotNetInfoParser

#include "stdafx.h"
#include "DotNetInfoParser.h"


// CDotNetInfoParser

STDMETHODIMP CDotNetInfoParser::Parse(VARIANT *v, IVariantObject** ppVariantObject)
{
	CHECK_E_POINTER(v);
	if(v->vt != VT_BSTR)
		return E_INVALIDARG;

	CComPtr<IPluginManager> pPluginManager;
	RETURN_IF_FAILED(HrGetPluginManager(&pPluginManager));
	RETURN_IF_FAILED(pPluginManager->CoCreateInstance(CLSID_VariantObject, IID_IVariantObject, (LPVOID*)ppVariantObject));
	CComPtr<IVariantObject> pVariantObject = *ppVariantObject;

	RETURN_IF_FAILED(HrAddColumn(pPluginManager, pVariantObject, VAR_KEY, L"Property"));
	RETURN_IF_FAILED(HrAddColumn(pPluginManager, pVariantObject, VAR_VALUE, L"Value"));

	CComPtr<IObjCollection> pObjectCollection;
	RETURN_IF_FAILED(HrCoCreateInstance(CLSID_ObjectCollection, &pObjectCollection));
	
	CComBSTR bstrResult = v->bstrVal;
	CString strResult = bstrResult;
	int nTokenPos = 0;
	int nObjects = 0;

	while (nTokenPos >= 0)
	{
		CString strLine = strResult.Tokenize(L"\n", nTokenPos);
		static std::wregex regex(L" +([^:]+): +(.+)");
		std::wsmatch matches;
		auto str = std::wstring(strLine);
		if (!std::regex_search(str, matches, regex) || matches.size() != 3)
			continue;

		int index = 1;
		auto strKey = matches[index++].str();
		auto strValue = matches[index++].str();

		CComPtr<IVariantObject> pVariantObjectValue;
		RETURN_IF_FAILED(pPluginManager->CoCreateInstance(CLSID_VariantObject, IID_IVariantObject, (LPVOID*)&pVariantObjectValue));
		RETURN_IF_FAILED(pVariantObjectValue->SetVariantValue(VAR_KEY, &CComVar(strKey.c_str())));
		RETURN_IF_FAILED(pVariantObjectValue->SetVariantValue(VAR_VALUE, &CComVar(strValue.c_str())));
		RETURN_IF_FAILED(pObjectCollection->AddObject(pVariantObjectValue));
		nObjects++;
	}

	if(!nObjects)
		return E_FAIL;

	CComPtr<IVariantTable> pVariantTable;
	RETURN_IF_FAILED(HrWrapToVariantTable(pPluginManager, pVariantObject, pObjectCollection, &pVariantTable));
	CComVar vObjects(pVariantTable);
	RETURN_IF_FAILED(pVariantObject->SetVariantValue(ObjectModel::Metadata::TableObject::ObjectsObject, &vObjects));
	RETURN_IF_FAILED(pVariantObject->SetVariantValue(VAR_RESULT, v));
	return S_OK;
}