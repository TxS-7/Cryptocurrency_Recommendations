LSH_DIR   = LSH
LSH_OBJS  = $(LSH_DIR)/LSH.o $(LSH_DIR)/hash_table.o $(LSH_DIR)/cosine_hash_table.o
OBJS      = tweet.o recommendation.o cosine_lsh_recommender.o clustering_recommender.o clustering.o data_point.o file_io.o util.o metrics.o
CC        = g++
FLAGS     = -Wall -g3 -std=c++11

all: recommendation best_clusters

recommendation: $(LSH_OBJS) $(OBJS) main.o
	$(CC) -o recommendation $(LSH_OBJS) $(OBJS) main.o


main.o: main.cpp tweet.h recommendation.h file_io.h util.h
	$(CC) $(FLAGS) -c main.cpp


best_clusters: $(LSH_OBJS) $(OBJS) best_clusters.o
	$(CC) -o best_clusters $(LSH_OBJS) $(OBJS) best_clusters.o


best_clusters.o: best_clusters.cpp tweet.h recommendation.h clustering_recommender.h file_io.h
	$(CC) $(FLAGS) -c best_clusters.cpp



recommendation.o: recommendation.cpp recommendation.h tweet.h clustering.h cosine_lsh_recommender.h data_point.h metrics.h
	$(CC) $(FLAGS) -c recommendation.cpp

cosine_lsh_recommender.o: cosine_lsh_recommender.cpp cosine_lsh_recommender.h $(LSH_DIR)/LSH.h data_point.h metrics.h util.h
	$(CC) $(FLAGS) -c cosine_lsh_recommender.cpp

clustering_recommender.o: clustering_recommender.cpp clustering_recommender.h clustering.h data_point.h metrics.h util.h
	$(CC) $(FLAGS) -c clustering_recommender.cpp



clustering.o: clustering.cpp clustering.h data_point.h metrics.h
	$(CC) $(FLAGS) -c clustering.cpp


$(LSH_DIR)/LSH.o: $(LSH_DIR)/LSH.cpp $(LSH_DIR)/LSH.h $(LSH_DIR)/hash_table.h data_point.h  $(LSH_DIR)/cosine_hash_table.h metrics.h
	$(CC) $(FLAGS) -c $(LSH_DIR)/LSH.cpp -o $(LSH_DIR)/LSH.o

$(LSH_DIR)/cosine_hash_table.o: $(LSH_DIR)/cosine_hash_table.cpp $(LSH_DIR)/cosine_hash_table.h $(LSH_DIR)/hash_table.h data_point.h metrics.h util.h
	$(CC) $(FLAGS) -c $(LSH_DIR)/cosine_hash_table.cpp -o $(LSH_DIR)/cosine_hash_table.o

$(LSH_DIR)/hash_table.o: $(LSH_DIR)/hash_table.cpp $(LSH_DIR)/hash_table.h data_point.h metrics.h
	$(CC) $(FLAGS) -c $(LSH_DIR)/hash_table.cpp -o $(LSH_DIR)/hash_table.o



file_io.o: file_io.cpp file_io.h tweet.h util.h
	$(CC) $(FLAGS) -c file_io.cpp

metrics.o: metrics.cpp metrics.h data_point.h
	$(CC) $(FLAGS) -c metrics.cpp

data_point.o: data_point.cpp data_point.h util.h metrics.h
	$(CC) $(FLAGS) -c data_point.cpp

tweet.o: tweet.cpp tweet.h util.h
	$(CC) $(FLAGS) -c tweet.cpp

util.o: util.cpp util.h
	$(CC) $(FLAGS) -c util.cpp



clean:
	rm -f $(OBJS) $(LSH_OBJS) main.o best_clusters.o recommendation best_clusters
