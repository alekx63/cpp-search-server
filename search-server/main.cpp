// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
	const int doc_id = 42;
	const string content = "cat in the city"s;
	const vector<int> ratings = { 1, 2, 3 };
	// Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
	// находит нужный документ
	{
		SearchServer server;
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		const auto found_docs = server.FindTopDocuments("in"s);
		//assert(found_docs.size() == 1);
		ASSERT_EQUAL(found_docs.size(), 1u);
		const Document & doc0 = found_docs[0];
		//assert(doc0.id == doc_id);
		ASSERT_EQUAL(doc0.id, doc_id);
	}

	// Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
	// возвращает пустой результат
	{
		SearchServer server;
		server.SetStopWords("in the"s);
		server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
		//assert(server.FindTopDocuments("in"s).empty());
		ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
	}
}

void TestAddDoc() {
	{
		SearchServer server;
		server.AddDocument(4, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
		//assert(server.GetDocumentCount() == 1);
		ASSERT_EQUAL(server.GetDocumentCount(), 1);
		vector<Document> found_docs = server.FindTopDocuments("белый"s);
		//assert(found_docs.size() == 1);
		ASSERT_EQUAL(found_docs.size(), 1);
		//assert(!found_docs.empty());
		ASSERT_HINT(!found_docs.empty(), "Stop words must be included in documents"s);
	}
	{
		SearchServer server;
		server.AddDocument(4, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
		server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
		//assert(server.GetDocumentCount() == 2);
		ASSERT_EQUAL(server.GetDocumentCount(), 2);
		vector<Document> found_docs = server.FindTopDocuments("кот"s);
		//assert(found_docs.size() == 2);
		ASSERT_EQUAL(found_docs.size(), 2);
		//assert(!found_docs.empty());
		ASSERT_HINT(!found_docs.empty(), "Stop words must be included in documents"s);
	}
}
void TestMinusWords() {
	SearchServer server;
	server.AddDocument(11, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
	server.AddDocument(22, "my live is my rulse"s, DocumentStatus::ACTUAL, { 5, -2, 3 });
	vector<Document> found_docs = server.FindTopDocuments("-in"s);
	//assert(found_docs.size() == 1);
	//assert(found_docs.empty());
	//assert(found_docs.size() == 1);
	//found_docs = server.FindTopDocuments("-in"s);
	//assert(found_docs.empty());
	ASSERT_HINT(found_docs.empty(), "Stop words must be excluded from documents"s);
}
void TestMatchDocument() {
	{
		SearchServer server;
		server.AddDocument(11, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		vector<string> v;
		DocumentStatus ds;
		tuple<vector<string>, DocumentStatus> md = server.MatchDocument("-the"s, 11);
		tie(v, ds) = md;
		//assert(v.size() == 0);
		ASSERT_EQUAL(v.size(), 0);
	}
	{
		SearchServer server;
		server.AddDocument(11, "cat in the city"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
		vector<string> v;
		DocumentStatus ds;
		tuple<vector<string>, DocumentStatus> md = server.MatchDocument("the"s, 11);
		tie(v, ds) = md;
		//assert(v.size() == 1);
		ASSERT_EQUAL(v.size(), 1);
															  
	}
}
void TestRelev() {

	const double EPSILON = 1e-6;
	SearchServer server;
	string word = "кот"s;
	server.AddDocument(4, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
	vector <vector<string>> documents = {
		{"белый"s, "кот"s, "и"s, "модный"s, "ошейник"s},
		{"пушистый"s, "кот"s, "пушистый"s, "хвост"s},
		{"ухоженный"s, "пёс"s, "выразительные"s, "глаза"s},
		{"ухоженный"s, "скворец"s, "евгений"s} };

	vector<double> tf, tf_idf;
	unsigned query_in_doc_c{}, query_count{};
	for (auto document : documents) {
		query_count = count(document.begin(), document.end(), word);
		query_in_doc_c += bool(query_count);
		tf.push_back(query_count / static_cast<double>(document.size()));
	}
	double IDF = log(documents.size() / static_cast<double>(query_in_doc_c ? query_in_doc_c : 1));
	for (auto tf_i : tf) {
		double d = IDF * tf_i;
		if (d > 0) tf_idf.push_back(IDF * tf_i);
	}
	//sort(tf_idf.begin(), tf_idf.end(), [EPSILON](double& lhs, double& rhs) {
		//return (lhs - rhs) > EPSILON; });
	sort(begin(tf_idf), end(tf_idf), [](double& d1, double& d2) {return d1 > d2; });
	vector <Document> nv = server.FindTopDocuments(word);
	for (int i = 0; i < tf_idf.size(); i++)
		//assert((tf_idf[i] - nv[i].relevance) < EPSILON);
		ASSERT((tf_idf[i] - nv[i].relevance) < EPSILON);
}
 
void TestRating() {

	SearchServer server;
	server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

	vector<vector<int>> ratings = { { 8, -3 },
									{ 7, 2, 7 },
									{ 5, -12, 2, 1 },
									{ 9 } };
	string word = "кот"s;
	vector<Document> MyVector;
	int i = 0;

	for (auto a : ratings) {
		int rating_sum = 0;

		for (const int rating : a) {
			rating_sum += rating;
		}
		int sr_ar = rating_sum / static_cast<int>(a.size());

		MyVector.push_back({ i, 0, sr_ar });
		i++;
	}

	vector<Document> testvec = server.FindTopDocuments(word);

	for (int j = 0; j < testvec.size(); j++) {
		for (int j2 = 0; j2 < MyVector.size(); j2++) {
			if (MyVector[j2].id == testvec[j].id) {
				//assert(MyVector[j2].rating == testvec[j].rating);
				ASSERT_EQUAL(MyVector[j2].rating, testvec[j].rating);
			}
		}
	}
}
void TestForPredicate() {
	{
		SearchServer server;
		const DocumentStatus status = DocumentStatus::ACTUAL;
		const vector<int> rating = { 1,2,3,4,5 };
		double sum_ratings = 0.0;
		for (int i = 0; i < rating.size(); i++)
		{
			sum_ratings += rating[i];
		}
		const int avg_rating = sum_ratings / rating.size();
		vector < string> docus =
		{
			"I like cats",
			"cats is nice animals",
			"Where are you from",
			"Who it can be now",
		};
		vector <vector<string>> documents =
		{
			{"I", "like", "cats"},
			{"cats", "is", "nice", "animals"},
			{"Where", "are", "you", "from"},
			{"Who", "it", "can", "be", "now"}
		};
		for (size_t i = 0; i < docus.size(); ++i)
		{
			server.AddDocument(i, docus[i], status, rating);
		}
		for (int i = 0; i < documents.size(); ++i) {
			for (const auto& s : documents[i]) {
				auto predicate = [i, status, avg_rating](int id, DocumentStatus st, int rating) {
					return id == i && st == status && avg_rating == rating;
				};
				auto search_results = server.FindTopDocuments(s, predicate);
 
 
				//assert(search_results.size() == 1);
				ASSERT_EQUAL(search_results.size(), 1);
			}
		}
	}
}
void TestForDocumentsStatus() {
	SearchServer server;
	const string content = "белый кот и модный ошейник"s;

	server.AddDocument(4, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
	server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

	auto t = server.FindTopDocuments("ухоженный"s, DocumentStatus::BANNED);
	//assert(t.size() == 1);
	ASSERT_EQUAL(t.size(), 1);
}


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
	TestExcludeStopWordsFromAddedDocumentContent();
	TestMatchDocument();
	TestAddDoc();
	TestMinusWords();
	TestRelev();
	TestRating();
	TestForPredicate();
	TestForDocumentsStatus();												 
}
