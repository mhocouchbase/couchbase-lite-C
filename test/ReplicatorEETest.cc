//
// ReplicatorEETest.cc
//
// Copyright © 2020 Couchbase. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "ReplicatorTest.hh"
#include "CBLPrivate.h"
#include <string>


#ifdef COUCHBASE_ENTERPRISE     // Local-to-local replication is an EE feature


class ReplicatorLocalTest : public ReplicatorTest {
public:
    Database otherDB;

    ReplicatorLocalTest()
    :otherDB(openEmptyDatabaseNamed("otherDB"))
    {
        config.endpoint = CBLEndpoint_CreateWithLocalDB(otherDB.ref());
    }
};


TEST_CASE_METHOD(ReplicatorLocalTest, "Replicate empty db", "[Replicator]") {
    config.replicatorType = kCBLReplicatorTypePush;
    replicate();
    CBLReplicatorStatus status = CBLReplicator_Status(repl);
    CHECK(status.error.code == 0);
    CHECK(status.progress.complete == 1.0);
    CHECK(status.progress.documentCount == 0);
    
    config.replicatorType = kCBLReplicatorTypePull;
    resetReplicator();
    replicate();
    status = CBLReplicator_Status(repl);
    CHECK(status.error.code == 0);
    CHECK(status.progress.complete == 1.0);
    CHECK(status.progress.documentCount == 0);
    
    config.replicatorType = kCBLReplicatorTypePushAndPull;
    resetReplicator();
    replicate();
    status = CBLReplicator_Status(repl);
    CHECK(status.error.code == 0);
    CHECK(status.progress.complete == 1.0);
    CHECK(status.progress.documentCount == 0);
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Push to local db", "[Replicator]") {
    config.replicatorType = kCBLReplicatorTypePush;
    
    MutableDocument doc("foo");
    doc["greeting"] = "Howdy!";
    db.saveDocument(doc);

    replicate();

    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});

    Document copiedDoc = otherDB.getDocument("foo");
    REQUIRE(copiedDoc);
    CHECK(copiedDoc["greeting"].asString() == "Howdy!"_sl);
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Continuous Push to local db", "[Replicator]") {
    config.replicatorType = kCBLReplicatorTypePush;
    config.continuous = true;
    
    MutableDocument doc("foo");
    doc["greeting"] = "Howdy!";
    db.saveDocument(doc);

    replicate();

    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});

    Document copiedDoc = otherDB.getDocument("foo");
    REQUIRE(copiedDoc);
    CHECK(copiedDoc["greeting"].asString() == "Howdy!"_sl);
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Set Suspended", "[Replicator]") {
    config.replicatorType = kCBLReplicatorTypePush;
    config.continuous = true;
    idleAction = IdleAction::kFinishMonitor;
    
    replicate();
    
    REQUIRE(CBLReplicator_Status(repl).activity == kCBLReplicatorIdle);
    CBLReplicator_SetSuspended(repl, true);
    REQUIRE(waitForActivityLevel(kCBLReplicatorOffline, 10.0));
    
    CBLReplicator_SetSuspended(repl, false);
    REQUIRE(waitForActivityLevel(kCBLReplicatorIdle, 10.0));
    
    CBLReplicator_Stop(repl);
    REQUIRE(waitForActivityLevel(kCBLReplicatorStopped, 10.0));
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Pending Documents", "[Replicator]") {
    config.replicatorType = kCBLReplicatorTypePush;
    
    replicate();
    CHECK(asVector(replicatedDocIDs) == vector<string>{});
    
    CBLError error;
    FLDict ids = CBLReplicator_PendingDocumentIDs(repl, &error);
    CHECK(FLDict_Count(ids) == 0);
    FLDict_Release(ids);
    
    MutableDocument doc1("foo1");
    doc1["greeting"] = "Howdy!";
    db.saveDocument(doc1);
    
    MutableDocument doc2("foo2");
    doc2["greeting"] = "Hello!";
    db.saveDocument(doc2);
    
    ids = CBLReplicator_PendingDocumentIDs(repl, &error);
    CHECK(FLDict_Count(ids) == 2);
    CHECK(FLDict_Get(ids, "foo1"_sl));
    CHECK(FLDict_Get(ids, "foo2"_sl));
    FLDict_Release(ids);
    
    CHECK(CBLReplicator_IsDocumentPending(repl, "foo1"_sl, &error));
    CHECK(CBLReplicator_IsDocumentPending(repl, "foo2"_sl, &error));
    
    replicate();
    
    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo1", "foo2"});
    
    ids = CBLReplicator_PendingDocumentIDs(repl, &error);
    CHECK(FLDict_Count(ids) == 0);
    FLDict_Release(ids);
    
    CHECK(!CBLReplicator_IsDocumentPending(repl, "foo1"_sl, &error));
    CHECK(error.code == 0);
    
    CHECK(!CBLReplicator_IsDocumentPending(repl, "foo2"_sl, &error));
    CHECK(error.code == 0);
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Default Resolver : Deleted Wins", "[Replicator][Conflict]") {
    SECTION("No conflict resolved specified") {
        config.conflictResolver = nullptr;
    }
    
    SECTION("Specify default conflict resolver") {
        config.conflictResolver = CBLDefaultConflictResolver;
    }
    
    config.replicatorType = kCBLReplicatorTypePull;
    
    // Delete Doc:
    MutableDocument doc("foo");
    doc["greeting"] = "Howdy!";
    db.saveDocument(doc);
    db.deleteDocument(doc);

    // Update multiple times:
    MutableDocument doc2("foo");
    doc2["greeting"] = "Salaam Alaykum";
    otherDB.saveDocument(doc2);

    doc2["greeting"] = "Hello";
    otherDB.saveDocument(doc2);
    
    doc2["greeting"] = "Konichiwa";
    otherDB.saveDocument(doc2);
    
    // Pull:
    resetReplicator();
    replicate();

    // Deleted doc should win:
    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});
    Document localDoc = db.getDocument("foo");
    REQUIRE(!localDoc);
    
    // Push
    config.replicatorType = kCBLReplicatorTypePush;
    replicatedDocIDs.clear();
    resetReplicator();
    replicate();
    
    // Resolved doc should be pushed:
    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});
    Document remoteDoc = otherDB.getDocument("foo");
    REQUIRE(!remoteDoc);
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Default Resolver : Higher Gen Wins", "[Replicator][Conflict]") {
    SECTION("No conflict resolved specified") {
        config.conflictResolver = nullptr;
    }
    
    SECTION("Specify default conflict resolver") {
        config.conflictResolver = CBLDefaultConflictResolver;
    }
    
    config.replicatorType = kCBLReplicatorTypePull;

    // Create:
    MutableDocument doc("foo");
    doc["greeting"] = "Howdy!";
    db.saveDocument(doc);
    
    // Create and Update:
    MutableDocument doc2("foo");
    doc2["greeting"] = "Salaam Alaykum";
    otherDB.saveDocument(doc2);
    
    doc2["greeting"] = "Konichiwa";
    otherDB.saveDocument(doc2);
    
    REQUIRE(CBLDocument_Generation(doc2.ref()) > CBLDocument_Generation(doc.ref()));

    // Pull
    resetReplicator();
    replicate();

    // Higher generation should win:
    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});
    Document localDoc = db.getDocument("foo");
    REQUIRE(localDoc);
    CHECK(localDoc["greeting"].asString() == "Konichiwa"_sl);
    CHECK(localDoc.revisionID() == doc2.revisionID());
    
    // Push
    config.replicatorType = kCBLReplicatorTypePush;
    replicatedDocIDs.clear();
    resetReplicator();
    replicate();
    
    // Resolved doc, same as remote doc, should not be pushed.
    CHECK(asVector(replicatedDocIDs) == vector<string>{});
    Document remoteDoc = db.getDocument("foo");
    REQUIRE(remoteDoc);
    CHECK(remoteDoc["greeting"].asString() == "Konichiwa"_sl);
    CHECK(remoteDoc.revisionID() == doc2.revisionID());
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Default Resolver : Higher RevID Wins", "[Replicator][Conflict]") {
    SECTION("No conflict resolved specified") {
        config.conflictResolver = nullptr;
    }
    
    SECTION("Specify default conflict resolver") {
        config.conflictResolver = CBLDefaultConflictResolver;
    }
    
    config.replicatorType = kCBLReplicatorTypePull;

    // Create:
    MutableDocument doc("foo");
    doc["greeting"] = "Howdy!";
    db.saveDocument(doc);

    // Create:
    MutableDocument doc2("foo");
    doc2["greeting"] = "Salaam Alaykum";
    otherDB.saveDocument(doc2);
    
    REQUIRE(doc2.revisionID().compare(doc.revisionID()) > 0);
    
    // Pull
    resetReplicator();
    replicate();

    // Higher revOD should win:
    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});
    Document localDoc = db.getDocument("foo");
    REQUIRE(localDoc);
    CHECK(localDoc["greeting"].asString() == "Salaam Alaykum"_sl);
    CHECK(localDoc.revisionID() == doc2.revisionID());
    
    // Push
    config.replicatorType = kCBLReplicatorTypePush;
    replicatedDocIDs.clear();
    resetReplicator();
    replicate();
    
    // Resolved doc should be the same:
    // Resolved doc, same as remote doc, should not be pushed.
    CHECK(asVector(replicatedDocIDs) == vector<string>{});
    Document remoteDoc = db.getDocument("foo");
    REQUIRE(remoteDoc);
    CHECK(remoteDoc["greeting"].asString() == "Salaam Alaykum"_sl);
    CHECK(remoteDoc.revisionID() == doc2.revisionID());
}

class ReplicatorConflictTest : public ReplicatorLocalTest {
public:
    enum class ResolverMode { kLocalWins, kRemoteWins, kMerge, kMergeAutoID };
    
    bool deleteLocal {false}, deleteRemote {false}, deleteMerged {false};
    ResolverMode resolverMode = {ResolverMode::kLocalWins};
    
    bool resolverCalled {false};
    alloc_slice expectedLocalRevID, expectedRemoteRevID;
    
    string docID;
    unsigned count {0};
    
    // Can be called multiple times; different document id will be used each time.
    void testConflict(bool delLocal, bool delRemote, bool delMerged, ResolverMode resMode) {
        deleteLocal = delLocal;
        deleteRemote = delRemote;
        deleteMerged = delMerged;
        resolverMode = resMode;
        
        expectedLocalRevID = nullptr;
        expectedRemoteRevID = nullptr;
        
        docID = "doc" + to_string(++count);
        
        // Save the same doc to each db (will have the same revision),
        MutableDocument doc(docID);
        doc["greeting"] = "Howdy!";
        db.saveDocument(doc);
        if (deleteLocal) {
            db.deleteDocument(doc);
        } else {
            doc["expletive"] = "Shazbatt!";
            auto blob = Blob("text/plain"_sl, "Blob!"_sl);
            doc["signature"] = blob.properties();
            db.saveDocument(doc);
            expectedLocalRevID = doc.revisionID();
        }

        doc = MutableDocument(docID);
        doc["greeting"] = "Howdy!";
        otherDB.saveDocument(doc);
        if (deleteRemote) {
            otherDB.deleteDocument(doc);
        } else {
            doc["expletive"] = "Frak!";
            auto blob = Blob("text/plain"_sl, "Pop!"_sl);
            doc["signature"] = blob.properties();
            otherDB.saveDocument(doc);
            expectedRemoteRevID = CBLDocument_CanonicalRevisionID(doc.ref());
        }

        replicatedDocIDs.clear();
        resolverCalled = false;
        
        config.conflictResolver = [](void *context,
                                     FLString documentID,
                                     const CBLDocument *localDocument,
                                     const CBLDocument *remoteDocument) -> const CBLDocument* {
            cerr << "--- Entering custom conflict resolver! (local=" << localDocument <<
                    ", remote=" << remoteDocument << ")\n";
            auto resolved = ((ReplicatorConflictTest*)context)->conflictResolver(documentID, localDocument, remoteDocument);
            cerr << "--- Returning " << resolved << " from custom conflict resolver\n";
            return resolved;
        };
        
        // Pull and Resolve Conflict:
        config.replicatorType = kCBLReplicatorTypePull;
        resetReplicator();
        replicate();

        // Check:
        CHECK(asVector(replicatedDocIDs) == vector<string>{docID});
        
        Document localDoc = db.getDocument(docID);
        if (resolverMode == ResolverMode::kLocalWins) {
            if (deleteLocal) {
                REQUIRE(!localDoc);
            } else {
                REQUIRE(localDoc);
                CHECK(localDoc["greeting"].asString() == "Howdy!"_sl);
                CHECK(localDoc["expletive"].asString() == "Shazbatt!"_sl);
                Blob blob(localDoc["signature"].asDict());
                CHECK(blob.loadContent() == "Blob!"_sl);
            }
        } else if (resolverMode == ResolverMode::kRemoteWins) {
            if (deleteRemote) {
                REQUIRE(!localDoc);
            } else {
                REQUIRE(localDoc);
                CHECK(localDoc["greeting"].asString() == "Howdy!"_sl);
                CHECK(localDoc["expletive"].asString() == "Frak!"_sl);
                Blob blob(localDoc["signature"].asDict());
                CHECK(blob.loadContent() == "Pop!"_sl);
            }
        } else {
            if (deleteMerged) {
                REQUIRE(!localDoc);
            } else {
                REQUIRE(localDoc);
                CHECK(localDoc["greeting"].asString() == "Howdy!"_sl);
                CHECK(localDoc["expletive"].asString() == "Frak!"_sl);
                Blob blob(localDoc["signature"].asDict());
                CHECK(blob.loadContent() == "Bob!"_sl);
                Blob blob2(localDoc["signature2"].asDict());
                CHECK(blob2.loadContent() == "Bob!"_sl);
            }
        }
        
        // Push Resolved Doc to Remote Server:
        config.replicatorType = kCBLReplicatorTypePush;
        resetReplicator();
        replicate();
        
        Document remoteDoc = otherDB.getDocument(docID);
        if (resolverMode == ResolverMode::kLocalWins) {
            if (deleteLocal) {
                REQUIRE(!remoteDoc);
            } else {
                REQUIRE(remoteDoc);
                CHECK(remoteDoc["greeting"].asString() == "Howdy!"_sl);
                CHECK(remoteDoc["expletive"].asString() == "Shazbatt!"_sl);
                Blob blob(localDoc["signature"].asDict());
                CHECK(blob.loadContent() == "Blob!"_sl);
            }
        } else if (resolverMode == ResolverMode::kRemoteWins) {
            if (deleteRemote) {
                REQUIRE(!remoteDoc);
            } else {
                REQUIRE(remoteDoc);
                CHECK(remoteDoc["greeting"].asString() == "Howdy!"_sl);
                CHECK(remoteDoc["expletive"].asString() == "Frak!"_sl);
                Blob blob(localDoc["signature"].asDict());
                CHECK(blob.loadContent() == "Pop!"_sl);
            }
        } else {
            if (deleteMerged) {
                REQUIRE(!remoteDoc);
            } else {
                REQUIRE(remoteDoc);
                CHECK(remoteDoc["greeting"].asString() == "Howdy!"_sl);
                CHECK(remoteDoc["expletive"].asString() == "Frak!"_sl);
                Blob blob(localDoc["signature"].asDict());
                CHECK(blob.loadContent() == "Bob!"_sl);
                Blob blob2(localDoc["signature2"].asDict());
                CHECK(blob2.loadContent() == "Bob!"_sl);
            }
        }
    }


    const CBLDocument* conflictResolver(slice documentID,
                                        const CBLDocument *localDocument,
                                        const CBLDocument *remoteDocument)
    {
        CHECK(!resolverCalled);
        resolverCalled = true;

        CHECK(string(documentID) == docID);
        if (deleteLocal) {
            REQUIRE(!localDocument);
            REQUIRE(!expectedLocalRevID);
        } else {
            REQUIRE(localDocument);
            CHECK(string(CBLDocument_ID(localDocument)) == docID);
            CHECK(slice(CBLDocument_RevisionID(localDocument)) == expectedLocalRevID);
            Dict localProps(CBLDocument_Properties(localDocument));
            CHECK(localProps["greeting"].asString() == "Howdy!"_sl);
            CHECK(localProps["expletive"].asString() == "Shazbatt!"_sl);
        }
        if (deleteRemote) {
            REQUIRE(!remoteDocument);
            REQUIRE(!expectedRemoteRevID);
        } else {
            REQUIRE(remoteDocument);
            CHECK(string(CBLDocument_ID(remoteDocument)) == docID);
            CHECK(slice(CBLDocument_RevisionID(remoteDocument)) == expectedRemoteRevID);
            Dict remoteProps(CBLDocument_Properties(remoteDocument));
            CHECK(remoteProps["greeting"].asString() == "Howdy!"_sl);
            CHECK(remoteProps["expletive"].asString() == "Frak!"_sl);
        }
        
        if (deleteMerged) {
            REQUIRE(resolverMode == ResolverMode::kMerge);
            return nullptr;
        } else {
            switch (resolverMode) {
                case ResolverMode::kLocalWins:
                    return localDocument;
                case ResolverMode::kRemoteWins:
                    return remoteDocument;
                default:
                    // NOTE: The merged doc will be:
                    // {'greeting': 'Howdy!', 'expletive': 'Frak!', 'signature': blob, 'signature2': blob}
                    // ** signature and signature2 have the same blob
                    
                    CBLDocument *mergedDoc;
                    if (resolverMode == ResolverMode::kMergeAutoID)
                        mergedDoc = CBLDocument_Create(); // Allowed but there will be a warning message
                    else
                        mergedDoc = CBLDocument_CreateWithID(documentID);
       
                    FLMutableDict mergedProps;
                    if (localDocument) {
                        FLDict props = CBLDocument_Properties(localDocument);
                        FLMutableDict mProps = FLDict_MutableCopy(props, kFLDefaultCopy);
                        CBLDocument_SetProperties(mergedDoc, mProps);
                        FLMutableDict_Release(mProps);
                        mergedProps = CBLDocument_MutableProperties(mergedDoc);
                    } else if (remoteDocument) {
                        FLDict props = CBLDocument_Properties(remoteDocument);
                        FLMutableDict mProps = FLDict_MutableCopy(props, kFLDefaultCopy);
                        CBLDocument_SetProperties(mergedDoc, mProps);
                        FLMutableDict_Release(mProps);
                        mergedProps = CBLDocument_MutableProperties(mergedDoc);
                    } else {
                        mergedProps = CBLDocument_MutableProperties(mergedDoc);
                        FLMutableDict_SetString(mergedProps, "greeting"_sl, "Howdy!"_sl);
                    }
                    
                    if (remoteDocument) {
                        FLDict props = CBLDocument_Properties(remoteDocument);
                        FLValue expletive = FLDict_Get(props, "expletive"_sl);
                        FLMutableDict_SetValue(mergedProps, "expletive"_sl, expletive);
                    } else {
                        FLMutableDict_SetString(mergedProps, "expletive"_sl, "Frak!"_sl);
                    }
                    
                    CBLBlob* blob = CBLBlob_CreateWithData("text/plain"_sl, "Bob!"_sl);
                    FLMutableDict_SetBlob(mergedProps, "signature"_sl, blob);
                    FLMutableDict_SetBlob(mergedProps, "signature2"_sl, blob); // Use same blob
                    
                    // Do not release `mergedDoc`, otherwise it would be freed before returning!
                    return mergedDoc;
            }
        }
    }
};


TEST_CASE_METHOD(ReplicatorConflictTest, "Custom resolver : local wins", "[Replicator][Conflict]") {
    testConflict(false, false, false, ResolverMode::kLocalWins);
    testConflict(false, true, false, ResolverMode::kLocalWins); // Remote deletion
    testConflict(true, false, false, ResolverMode::kLocalWins); // Local deletion
}


TEST_CASE_METHOD(ReplicatorConflictTest, "Custom resolver : remote wins", "[Replicator][Conflict]") {
    testConflict(false, false, false, ResolverMode::kRemoteWins);
    testConflict(false, true, false, ResolverMode::kRemoteWins); // Remote deletion
    testConflict(true, false, false, ResolverMode::kRemoteWins); // Local deletion
}


TEST_CASE_METHOD(ReplicatorConflictTest, "Custom resolver : merge", "[Replicator][Conflict]") {
    testConflict(false, false, false, ResolverMode::kMerge);
    testConflict(false, false, false, ResolverMode::kMergeAutoID);
    testConflict(false, true, false, ResolverMode::kMerge); // Remote deletion
    testConflict(true, false, false, ResolverMode::kMerge); // Local deletion
    testConflict(false, false, true, ResolverMode::kMerge); // Merge deletion
}


TEST_CASE_METHOD(ReplicatorLocalTest, "Document Replication Listener", "[Replicator]") {
    // No listener:
    MutableDocument doc("foo");
    doc["greeting"] = "Howdy!";
    db.saveDocument(doc);
    
    config.replicatorType = kCBLReplicatorTypePush;
    enableDocReplicationListener = false;
    replicate();
    CHECK(replicatedDocIDs.empty());
    
    // Add listener:
    doc["greeting"] = "Hello!";
    db.saveDocument(doc);
    enableDocReplicationListener = true;
    replicate();
    CHECK(asVector(replicatedDocIDs) == vector<string>{"foo"});
    
    // Remove listener:
    doc["greeting"] = "Hi!";
    db.saveDocument(doc);
    enableDocReplicationListener = false;
    replicatedDocIDs.clear();
    replicate();
    CHECK(replicatedDocIDs.empty());
}


class ReplicatorFilterTest : public ReplicatorLocalTest {
public:
    int count = 0;
    int deletedCount = 0;
    alloc_slice deletedDocID;
    bool rejectAll = false;
    
    void testFilter(bool isPush, bool rejectAllChanges) {
        cbl::Database theDB;
        
        if (isPush) {
            theDB = db;
            config.replicatorType = kCBLReplicatorTypePush;
            config.pushFilter = [](void *context, CBLDocument* doc, CBLDocumentFlags flags) -> bool {
                ReplicatorFilterTest* test = ((ReplicatorFilterTest*)context);
                return test->filter(context, doc, flags);
            };
        } else {
            theDB = otherDB;
            config.replicatorType = kCBLReplicatorTypePull;
            config.pullFilter = [](void *context, CBLDocument* doc, CBLDocumentFlags flags) -> bool {
                ReplicatorFilterTest* test = ((ReplicatorFilterTest*)context);
                return test->filter(context, doc, flags);
            };
            
            // Make local db non-empty for pulling a deleted doc:
            MutableDocument doc0("doc0");
            db.saveDocument(doc0);
        }
        
        MutableDocument doc1("doc1");
        theDB.saveDocument(doc1);
        
        MutableDocument doc2("doc2");
        theDB.saveDocument(doc2);
        
        MutableDocument doc3("doc3");
        theDB.saveDocument(doc3);
        theDB.deleteDocument(doc3);
        
        count = 0;
        deletedCount = 0;
        deletedDocID = nullptr;
        rejectAll = rejectAllChanges;
        
        resetReplicator();
        replicate();
        
        CBLReplicatorStatus status = CBLReplicator_Status(repl);
        CHECK(status.error.code == 0);
        CHECK(status.progress.complete == 1.0);
        CHECK(status.progress.documentCount == (rejectAllChanges ? 0 : 2));
        
        CHECK(count == 3);
        CHECK(deletedCount == 1);
        CHECK(deletedDocID == "doc3"_sl);
    }
    
    bool filter(void *context, CBLDocument* doc, CBLDocumentFlags flags) {
        count++;
        
        if (flags & kCBLDocumentFlagsDeleted) {
            deletedCount++;
            deletedDocID = CBLDocument_ID(doc);
        }
        
        if (rejectAll || FLSlice_Equal(CBLDocument_ID(doc), "doc2"_sl))
            return false;
        
        return true;
    }
};


TEST_CASE_METHOD(ReplicatorFilterTest, "Push Filter", "[Replicator][Filter]") {
    testFilter(true, true);
    testFilter(true, false);
}


TEST_CASE_METHOD(ReplicatorFilterTest, "Pull Filter", "[Replicator][Filter]") {
    testFilter(false, true);
    testFilter(false, false);
}


#endif // COUCHBASE_ENTERPRISE
