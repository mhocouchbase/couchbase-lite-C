CBL_C {
	global:
		CBL_Init;
		CBLEncryptionKey_FromPassword;
		CBLDatabase_ChangeEncryptionKey;
		CBLEndpoint_CreateWithLocalDB;
		kCBLEncryptableType;
		kCBLEncryptableValueProperty;
		CBLEncryptable_CreateWithNull;
		CBLEncryptable_CreateWithBool;
		CBLEncryptable_CreateWithInt;
		CBLEncryptable_CreateWithUInt;
		CBLEncryptable_CreateWithFloat;
		CBLEncryptable_CreateWithDouble;
		CBLEncryptable_CreateWithString;
		CBLEncryptable_CreateWithValue;
		CBLEncryptable_CreateWithArray;
		CBLEncryptable_CreateWithDict;
		CBLEncryptable_Properties;
		CBLEncryptable_Value;
		FLDict_IsEncryptableValue;
		FLDict_GetEncryptableValue;
		FLSlot_SetEncryptableValue;
		CBL_Retain;
		CBL_Release;
		CBL_InstanceCount;
		CBL_DumpInstances;
		CBL_Now;
		CBLError_Message;
		CBLError_GetCaptureBacktraces;
		CBLError_SetCaptureBacktraces;
		CBLListener_Remove;
		kCBLTypeProperty;
		kCBLBlobType;
		kCBLBlobDigestProperty;
		kCBLBlobLengthProperty;
		kCBLBlobContentTypeProperty;
		FLDict_IsBlob;
		FLDict_GetBlob;
		FLSlot_SetBlob;
		CBLBlob_ContentType;
		CBLBlob_Length;
		CBLBlob_Digest;
		CBLBlob_Equals;
		CBLBlob_Properties;
		CBLBlob_Content;
		CBLBlob_OpenContentStream;
		CBLBlob_CreateJSON;
		CBLBlob_CreateWithData;
		CBLBlob_CreateWithStream;
		CBLBlobReader_Read;
		CBLBlobReader_Close;
		CBLBlobWriter_Create;
		CBLBlobWriter_Close;
		CBLBlobWriter_Write;
		CBLDatabase_GetBlob;
		CBLDatabase_SaveBlob;
		CBLDatabaseConfiguration_Default;
		CBL_DatabaseExists;
		CBL_CopyDatabase;
		CBL_DeleteDatabase;
		CBLDatabase_Delete;
		CBLDatabase_Open;
		CBLDatabase_Close;
		CBLDatabase_Name;
		CBLDatabase_Path;
		CBLDatabase_Config;
		CBLDatabase_Count;
		CBLDatabase_LastSequence;
		CBLDatabase_Delete;
		CBLDatabase_BeginTransaction;
		CBLDatabase_EndTransaction;
		CBLDatabase_PerformMaintenance;
		CBLDatabase_AddChangeListener;
		CBLDatabase_AddChangeDetailListener;
		CBLDatabase_AddDocumentChangeListener;
		CBLDatabase_BufferNotifications;
		CBLDatabase_SendNotifications;
		CBLDatabase_CreateValueIndex;
		CBLDatabase_CreateFullTextIndex;
		CBLDatabase_DeleteIndex;
		CBLDatabase_GetIndexNames;
		CBLDocument_ID;
		CBLDocument_RevisionID;
		CBLDocument_CanonicalRevisionID;
		CBLDocument_Generation;
		CBLDocument_Sequence;
		CBLDocument_Create;
		CBLDocument_CreateWithID;
		CBLDocument_MutableCopy;
		CBLDocument_Properties;
		CBLDocument_MutableProperties;
		CBLDocument_SetProperties;
		CBLDocument_CreateJSON;
		CBLDocument_SetJSON;
		CBLDatabase_GetDocument;
		CBLDatabase_GetMutableDocument;
		CBLDatabase_SaveDocument;
		CBLDatabase_SaveDocumentWithConcurrencyControl;
		CBLDatabase_SaveDocumentWithConflictHandler;
		CBLDatabase_DeleteDocument;
		CBLDatabase_DeleteDocumentWithConcurrencyControl;
		CBLDatabase_DeleteDocumentByID;
		CBLDatabase_PurgeDocument;
		CBLDatabase_PurgeDocumentByID;
		CBLDatabase_GetDocumentExpiration;
		CBLDatabase_SetDocumentExpiration;
		CBL_Log;
		CBL_LogMessage;
		CBLLog_Callback;
		CBLLog_SetCallback;
		CBLLog_CallbackLevel;
		CBLLog_SetCallbackLevel;
		CBLLog_ConsoleLevel;
		CBLLog_SetConsoleLevel;
		CBLLog_FileConfig;
		CBLLog_SetFileConfig;
		CBLLog_BeginExpectingExceptions;
		CBLLog_EndExpectingExceptions;
		CBLDatabase_CreateQuery;
		CBLQuery_Parameters;
		CBLQuery_SetParameters;
		CBLQuery_Execute;
		CBLQuery_Explain;
		CBLQuery_ColumnCount;
		CBLQuery_ColumnName;
		CBLQuery_AddChangeListener;
		CBLQuery_CopyCurrentResults;
		CBLResultSet_Next;
		CBLResultSet_ValueAtIndex;
		CBLResultSet_ValueForKey;
		CBLResultSet_ResultArray;
		CBLResultSet_ResultDict;
		CBLResultSet_GetQuery;
		kCBLAuthDefaultCookieName;
		CBLEndpoint_CreateWithURL;
		CBLEndpoint_Free;
		CBLAuth_CreatePassword;
		CBLAuth_CreateSession;
		CBLAuth_Free;
		CBLReplicator_Create;
		CBLReplicator_Config;
		CBLReplicator_Start;
		CBLReplicator_Stop;
		CBLReplicator_SetHostReachable;
		CBLReplicator_SetSuspended;
		CBLReplicator_Status;
		CBLReplicator_PendingDocumentIDs;
		CBLReplicator_IsDocumentPending;
		CBLReplicator_AddChangeListener;
		CBLReplicator_AddDocumentReplicationListener;
		CBLDefaultConflictResolver;
		kFLNullValue;
		kFLUndefinedValue;
		kFLEmptyArray;
		kFLEmptyDict;
		FLSlice_Equal;
		FLSlice_Compare;
		FLSlice_Hash;
		FLSlice_Copy;
		FLSlice_ToCString;
		FLSliceResult_New;
		_FLBuf_Retain;
		_FLBuf_Release;
		FLDoc_FromResultData;
		FLDoc_FromJSON;
		FLDoc_Release;
		FLDoc_Retain;
		FLDoc_GetAllocedData;
		FLDoc_GetData;
		FLDoc_GetRoot;
		FLDoc_GetSharedKeys;
		FLData_Dump;
		FLDump;
		FLDumpData;
		FLValue_FromData;
		FLValue_GetType;
		FLValue_IsInteger;
		FLValue_IsUnsigned;
		FLValue_IsDouble;
		FLValue_IsEqual;
		FLValue_AsBool;
		FLValue_AsData;
		FLValue_AsInt;
		FLValue_AsUnsigned;
		FLValue_AsFloat;
		FLValue_AsDouble;
		FLValue_AsString;
		FLValue_AsArray;
		FLValue_AsDict;
		FLValue_AsTimestamp;
		FLValue_ToString;
		FLValue_ToJSON;
		FLValue_ToJSONX;
		FLValue_ToJSON5;
		FLValue_FindDoc;
		FLValue_Retain;
		FLValue_Release;
		FLData_ConvertJSON;
		FLJSON5_ToJSON;
		FLArray_Count;
		FLArray_IsEmpty;
		FLArray_Get;
		FLArray_AsMutable;
		FLArray_MutableCopy;
		FLArrayIterator_Begin;
		FLArrayIterator_GetCount;
		FLArrayIterator_GetValue;
		FLArrayIterator_GetValueAt;
		FLArrayIterator_Next;
		FLMutableArray_New;
		FLMutableArray_GetSource;
		FLMutableArray_IsChanged;
		FLMutableArray_Append;
		FLMutableArray_Set;
		FLMutableArray_Insert;
		FLMutableArray_Remove;
		FLMutableArray_Resize;
		FLMutableArray_GetMutableArray;
		FLMutableArray_GetMutableDict;
		FLDict_Count;
		FLDict_IsEmpty;
		FLDict_Get;
		FLDict_GetWithKey;
		FLDict_AsMutable;
		FLDict_MutableCopy;
		FLDictIterator_Begin;
		FLDictIterator_GetCount;
		FLDictIterator_GetKey;
		FLDictIterator_GetKeyString;
		FLDictIterator_GetValue;
		FLDictIterator_Next;
		FLDictIterator_End;
		FLDictKey_Init;
		FLDictKey_GetString;
		FLEncoder_New;
		FLEncoder_NewWithOptions;
		FLEncoder_NewWritingToFile;
		FLEncoder_Free;
		FLEncoder_SetSharedKeys;
		FLEncoder_GetBase;
		FLEncoder_SuppressTrailer;
		FLEncoder_WriteRaw;
		FLEncoder_GetNextWritePos;
		FLEncoder_FinishItem;
		FLEncoder_Amend;
		FLEncoder_WriteNull;
		FLEncoder_WriteUndefined;
		FLEncoder_WriteBool;
		FLEncoder_WriteInt;
		FLEncoder_WriteUInt;
		FLEncoder_WriteFloat;
		FLEncoder_WriteDouble;
		FLEncoder_WriteString;
		FLEncoder_WriteDateString;
		FLEncoder_WriteData;
		FLEncoder_WriteValue;
		FLEncoder_ConvertJSON;
		FLEncoder_BeginArray;
		FLEncoder_EndArray;
		FLEncoder_BeginDict;
		FLEncoder_WriteKey;
		FLEncoder_WriteKeyValue;
		FLEncoder_EndDict;
		FLEncoder_BytesWritten;
		FLEncoder_FinishDoc;
		FLEncoder_Finish;
		FLEncoder_Reset;
		FLEncoder_GetError;
		FLEncoder_GetErrorMessage;
		FLCreateJSONDelta;
		FLEncodeJSONDelta;
		FLApplyJSONDelta;
		FLEncodeApplyingJSONDelta;
		FLMutableDict_New;
		FLMutableDict_GetSource;
		FLMutableDict_IsChanged;
		FLMutableDict_Set;
		FLMutableDict_Remove;
		FLMutableDict_RemoveAll;
		FLMutableDict_GetMutableArray;
		FLMutableDict_GetMutableDict;
		FLSlot_SetNull;
		FLSlot_SetBool;
		FLSlot_SetInt;
		FLSlot_SetUInt;
		FLSlot_SetFloat;
		FLSlot_SetDouble;
		FLSlot_SetString;
		FLSlot_SetData;
		FLSlot_SetValue;
		FLSharedKeys_Retain;
		FLSharedKeys_Release;
		FLSharedKeys_New;
		FLSharedKeys_LoadStateData;
		FLSharedKeys_LoadState;
		FLSharedKeys_GetStateData;
		FLSharedKeys_Count;
		FLSharedKeys_RevertToCount;
		FLSharedKeys_WriteState;
		FLKeyPath_New;
		FLKeyPath_Free;
		FLKeyPath_Eval;
		FLKeyPath_EvalOnce;
		FLKeyPath_ToString;
		FLKeyPath_Equals;
		FLDeepIterator_New;
		FLDeepIterator_Free;
		FLDeepIterator_GetValue;
		FLDeepIterator_GetKey;
		FLDeepIterator_GetIndex;
		FLDeepIterator_GetDepth;
		FLDeepIterator_SkipChildren;
		FLDeepIterator_Next;
		FLDeepIterator_GetPath;
		FLDeepIterator_GetPathString;
		FLDeepIterator_GetJSONPointer;
	local:
		*;
};
