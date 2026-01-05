#include <iostream>
#include <string>
#include <vector>
#include <ctime>

using namespace std;

//==============================================================================
// CUSTOM UTILITY FUNCTIONS
// Purpose: Basic implementations to avoid including extra standard libraries
//          like <algorithm>, adhering to from-scratch requirements.
//==============================================================================

/**
 *  Swaps two values of any type.
 */
template<typename T>
void custom_swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

/**
 *  Reverses a vector in place.
 */
template<typename T>
void custom_reverse(vector<T>& vec) {
    if (vec.empty()) return;
    size_t start = 0;
    size_t end = vec.size() - 1;
    while (start < end) {
        custom_swap(vec[start], vec[end]);
        start++;
        end--;
    }
}

//==============================================================================
// CUSTOM HASH FUNCTION
// Purpose: Provides a flexible hashing mechanism for the HashMap by using
//          template specialization for different key types.
//==============================================================================

template<typename K>
struct custom_hash;

/**
 *  Hash function specialization for integer keys.
 * Simply uses the integer value as its own hash code.
 */
template<>
struct custom_hash<int> {
    size_t operator()(const int& key) const {
        return static_cast<size_t>(key);
    }
};

/**
 *  Hash function specialization for string keys.
 * Implements a polynomial rolling hash for effective distribution.
 */
template<>
struct custom_hash<string> {
    size_t operator()(const string& key) const {
        size_t hash_val = 0;
        const int p = 31; // A small prime number for the polynomial base.
        long long p_pow = 1;
        for (char c : key) {
            hash_val = (hash_val + (c - 'a' + 1) * p_pow);
            p_pow = (p_pow * p);
        }
        return hash_val;
    }
};

//==============================================================================
// VERSION NODE
// Purpose: Represents a single version of a file in the version history tree.
//==============================================================================

struct VersionNode {
    int version_id;             // Unique identifier for this version within the file.
    string content;             // The content of the file at this version.
    string message;             // The snapshot message, if this version is a snapshot.
    time_t created_timestamp;   // Timestamp of when this version was created.
    time_t snapshot_timestamp;  // Timestamp of the snapshot; 0 if not a snapshot.
    VersionNode* parent;        // Pointer to the parent version in the tree.
    vector<VersionNode*> children; // Pointers to child versions (branches).

    /**
     *  Constructs a new version node.
     */
    VersionNode(int id, const string& initial_content, VersionNode* parent_node)
        : version_id(id),
          content(initial_content),
          parent(parent_node),
          created_timestamp(time(nullptr)),
          snapshot_timestamp(0), // Initially not a snapshot.
          message("") {}

    /**
     * Checks if this version is a snapshot.
     * True if it's a snapshot (timestamp is set), false otherwise.
     */
    bool isSnapshot() const {
        return snapshot_timestamp != 0;
    }
};

//==============================================================================
// HASH MAP
// Purpose: A custom HashMap implementation using separate chaining for collision
//          resolution. Provides O(1) average time complexity for lookups.
//==============================================================================

template <typename K, typename V>
class HashMap {
private:
    // Node structure for the linked list at each bucket.
    struct Node {
        K key;
        V value;
        Node* next;
        Node(K k, V v) : key(k), value(v), next(nullptr) {}
    };

    Node** table;       // The array of buckets (pointers to linked lists).
    int capacity;       // The total number of buckets.
    int current_size;   // The number of key-value pairs stored.

    /**
     *  Computes the bucket index for a given key.
     */
    int hashFunction(K key) const {
        return custom_hash<K>{}(key) % capacity;
    }

public:
    /**
     *  Constructs the HashMap with a given capacity.
     */
    HashMap(int initial_capacity = 16) {
        capacity = (initial_capacity > 0) ? initial_capacity : 16;
        current_size = 0;
        table = new Node*[capacity](); // Initialize all buckets to nullptr.
    }

    /**
     *  Destructor to clean up all dynamically allocated memory.
     */
    ~HashMap() {
        for (int i = 0; i < capacity; ++i) {
            Node* entry = table[i];
            while (entry != nullptr) {
                Node* prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
        delete[] table;
    }

    /**
     *  Inserts or updates a key-value pair.
     */
    void put(K key, V value) {
        int index = hashFunction(key);
        Node* entry = table[index];
        // Check if the key already exists to update it.
        while (entry != nullptr) {
            if (entry->key == key) {
                entry->value = value;
                return;
            }
            entry = entry->next;
        }
        // If key doesn't exist, create a new node at the front of the list.
        Node* newNode = new Node(key, value);
        newNode->next = table[index];
        table[index] = newNode;
        current_size++;
    }

    /**
     *  Retrieves the value for a given key.
     *  runtime_error if the key is not found.
     */
    V get(K key) const {
        int index = hashFunction(key);
        Node* entry = table[index];
        while (entry != nullptr) {
            if (entry->key == key) {
                return entry->value;
            }
            entry = entry->next;
        }
        throw runtime_error("Key not found in HashMap");
    }

    /**
     *  Checks if a key exists in the map.
     */
    bool containsKey(K key) const {
        int index = hashFunction(key);
        Node* entry = table[index];
        while (entry != nullptr) {
            if (entry->key == key) return true;
            entry = entry->next;
        }
        return false;
    }
    
    /**
     *  Returns a vector containing all values in the map.
     */
    vector<V> getValues() const {
        vector<V> values;
        for (int i = 0; i < capacity; ++i) {
            Node* entry = table[i];
            while(entry != nullptr) {
                values.push_back(entry->value);
                entry = entry->next;
            }
        }
        return values;
    }
};


//==============================================================================
// MAX HEAP & FILE METRIC
// Purpose: A MaxHeap for efficiently tracking system-wide file analytics, such
//          as the most recently modified files or files with the most versions.
//==============================================================================

/**
 *  A simple struct to hold file metrics for ranking in the MaxHeap.
 */
struct FileMetric {
    string filename;
    long long value; // Can represent timestamp or version count.

    // Comparison operator needed by the heap to order elements.
    bool operator<(const FileMetric& other) const {
        return this->value < other.value;
    }
};

/**
 *  A MaxHeap implementation using a vector as the underlying container.
 */
template <typename T>
class MaxHeap {
private:
    vector<T> heap;

    // Helper functions to get parent and child indices.
    int parent(int i) { return (i - 1) / 2; }
    int leftChild(int i) { return 2 * i + 1; }
    int rightChild(int i) { return 2 * i + 2; }

    /**
     *  Moves an element up the heap to maintain the heap property.
     */
    void heapifyUp(int index) {
        while (index > 0 && heap[parent(index)] < heap[index]) {
            custom_swap(heap[parent(index)], heap[index]);
            index = parent(index);
        }
    }

    /**
     *  Moves an element down the heap to maintain the heap property.
     */
    void heapifyDown(int index) {
        int maxIndex = index;
        while (true) {
            int l = leftChild(index);
            int r = rightChild(index);
            if (l < heap.size() && heap[maxIndex] < heap[l]) {
                maxIndex = l;
            }
            if (r < heap.size() && heap[maxIndex] < heap[r]) {
                maxIndex = r;
            }
            if (index == maxIndex) break; // Element is in its correct place.
            custom_swap(heap[index], heap[maxIndex]);
            index = maxIndex;
        }
    }

public:
    bool isEmpty() const { return heap.empty(); }

    /**
     *  Inserts a new value into the heap.
     */
    void insert(const T& value) {
        heap.push_back(value);
        heapifyUp(heap.size() - 1);
    }

    /**
     *  Extracts and returns the maximum value from the heap.
     *  out_of_range if the heap is empty.
     */
    T extractMax() {
        if (heap.empty()) throw out_of_range("Heap is empty");
        T maxValue = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return maxValue;
    }
};

//==============================================================================
// FILE CLASS
// Purpose: Manages the version history and metadata for a single file.
//==============================================================================

class File {
private:
    string filename;
    VersionNode* root;                  // The root of the version history tree.
    VersionNode* active_version;        // The currently active version (HEAD).
    HashMap<int, VersionNode*> version_map; // For O(1) lookup of versions by ID.
    int total_versions;                 // Counter for assigning new version IDs.
    time_t last_modification_time;      // Timestamp of the last modification.

    /**
     *  Recursively deletes the version tree to prevent memory leaks.
     */
    void deleteTree(VersionNode* node);

public:
    File(const string& name);
    ~File();

    string read() const;
    void insert(const string& content);
    void update(const string& content);
    void snapshot(const string& message);
    bool rollback(int versionId = -1);
    string history() const;

    // Accessors for file metadata.
    string getName() const;
    int getVersionCount() const;
    time_t getLastModificationTime() const;
};

//==============================================================================
// FILE SYSTEM CLASS
// Purpose: Acts as the main controller for the version control system,
//          managing all files and providing the user-facing interface.
//==============================================================================

class FileSystem {
private:
    HashMap<string, File*> files;          // Maps filenames to File objects.
    MaxHeap<FileMetric> recent_files_heap;  // Heap for tracking recently modified files.
    MaxHeap<FileMetric> biggest_trees_heap; // Heap for tracking files with the most versions.

    /**
     *  Rebuilds the analytics heaps. Called after any modification.
     * NOTE: This is inefficient for large systems but simple for this project.
     */
    void updateAnalytics();

public:
    FileSystem();
    ~FileSystem();

    // Core file operations.
    void create(const string& filename);
    string read(const string& filename);
    void insert(const string& filename, const string& content);
    void update(const string& filename, const string& content);
    void snapshot(const string& filename, const string& message);
    void rollback(const string& filename, int versionId = -1);
    string history(const string& filename);

    // System-wide analytics.
    string recentFiles(int num);
    string biggestTrees(int num);
};

//==============================================================================
// METHOD IMPLEMENTATIONS: File
//==============================================================================

File::File(const string& name) : filename(name), version_map(16) {
    total_versions = 1;
    root = new VersionNode(0, "", nullptr);
    active_version = root;
    // The root version is always an initial snapshot.
    root->message = "Initial version";
    root->snapshot_timestamp = time(nullptr);
    last_modification_time = root->snapshot_timestamp;
    version_map.put(0, root);
}

File::~File() {
    deleteTree(root);
}

void File::deleteTree(VersionNode* node) {
    if (node == nullptr) return;
    for (VersionNode* child : node->children) {
        deleteTree(child);
    }
    delete node;
}

string File::read() const {
    return active_version->content;
}

void File::insert(const string& content_to_add) {
    // Core versioning logic: if the current version is a snapshot, create a new
    // child version. Otherwise, modify the current (mutable) version in place.
    if (active_version->isSnapshot()) {
        string new_content = active_version->content + content_to_add;
        VersionNode* new_version = new VersionNode(total_versions, new_content, active_version);
        active_version->children.push_back(new_version);
        active_version = new_version;
        version_map.put(total_versions, new_version);
        total_versions++;
    } else {
        active_version->content += content_to_add;
    }
    last_modification_time = time(nullptr);
}

void File::update(const string& new_content) {
    // Versioning logic is identical to insert().
    if (active_version->isSnapshot()) {
        VersionNode* new_version = new VersionNode(total_versions, new_content, active_version);
        active_version->children.push_back(new_version);
        active_version = new_version;
        version_map.put(total_versions, new_version);
        total_versions++;
    } else {
        active_version->content = new_content;
    }
    last_modification_time = time(nullptr);
}

void File::snapshot(const string& message) {
    // Prevent creating a snapshot of an already snapshotted version.
    if (active_version->isSnapshot()) {
        cout << "Error: A snapshot already exists for the current version. "
                  << "Modify the file to create a new version before snapshotting.\n";
        return;
    }
    active_version->message = message;
    active_version->snapshot_timestamp = time(nullptr);
    last_modification_time = time(nullptr); // Snapshotting counts as a modification.
    cout << "Snapshot created for '" << filename << "'.\n";
}

bool File::rollback(int versionId) {
    // Case 1: Rollback to parent version.
    if (versionId == -1) {
        if (active_version->parent != nullptr) {
            active_version = active_version->parent;
            return true;
        }
        return false; // Already at the root, cannot go back further.
    // Case 2: Rollback to a specific version ID.
    } else {
        // Prevent a pointless rollback to the already active version.
        if (versionId == active_version->version_id) {
            cout << "Error: Cannot rollback to the version that is already active.\n";
            return false;
        }

        try {
            // Use the hash map for a fast O(1) lookup.
            VersionNode* target_version = version_map.get(versionId);
            active_version = target_version;
            return true;
        } catch (const runtime_error& e) {
            return false; // Version ID not found in the map.
        }
    }
}

string File::history() const {
    string result = "";
    VersionNode* current = active_version;
    vector<string> history_entries;
    // Traverse up the tree from the active node to the root.
    while (current != nullptr) {
        if (current->isSnapshot()) {
            char time_buf[100];
            strftime(time_buf, sizeof(time_buf), "%c", localtime(&current->snapshot_timestamp));
            string entry = "Version: " + to_string(current->version_id)
                              + ", Timestamp: " + time_buf
                              + ", Message: " + current->message;
            history_entries.push_back(entry);
        }
        current = current->parent;
    }
    // Reverse the list to display in chronological order (root first).
    custom_reverse(history_entries);
    for (const auto& entry : history_entries) {
        result += entry + "\n";
    }
    return result;
}

string File::getName() const { return filename; }
int File::getVersionCount() const { return total_versions; }
time_t File::getLastModificationTime() const { return last_modification_time; }


//==============================================================================
// METHOD IMPLEMENTATIONS: FileSystem
//==============================================================================

FileSystem::FileSystem() : files(256) {} // Initialize with a capacity of 256.

FileSystem::~FileSystem() {
    // Clean up dynamically allocated File objects.
    vector<File*> all_files = files.getValues();
    for (File* file_ptr : all_files) {
        delete file_ptr;
    }
}

void FileSystem::updateAnalytics() {
    // Re-create the heaps from scratch.
    recent_files_heap = MaxHeap<FileMetric>();
    biggest_trees_heap = MaxHeap<FileMetric>();
    vector<File*> all_files = files.getValues();
    // Populate heaps with fresh data from all files.
    for (File* file_ptr : all_files) {
        recent_files_heap.insert({file_ptr->getName(), file_ptr->getLastModificationTime()});
        biggest_trees_heap.insert({file_ptr->getName(), (long long)file_ptr->getVersionCount()});
    }
}

void FileSystem::create(const string& filename) {
    if (files.containsKey(filename)) {
        cout << "Error: File '" << filename << "' already exists.\n";
        return;
    }
    files.put(filename, new File(filename));
    updateAnalytics();
    cout << "File '" << filename << "' created.\n";
}

string FileSystem::read(const string& filename) {
    if (!files.containsKey(filename)) return "Error: File not found.";
    return files.get(filename)->read();
}

void FileSystem::insert(const string& filename, const string& content) {
    if (!files.containsKey(filename)) {
        cout << "Error: File not found.\n";
        return;
    }
    files.get(filename)->insert(content);
    updateAnalytics();
    cout << "Content inserted into '" << filename << "'.\n";
}

void FileSystem::update(const string& filename, const string& content) {
    if (!files.containsKey(filename)) {
        cout << "Error: File not found.\n";
        return;
    }
    files.get(filename)->update(content);
    updateAnalytics();
    cout << "Content updated in '" << filename << "'.\n";
}

void FileSystem::snapshot(const string& filename, const string& message) {
    if (!files.containsKey(filename)) {
        cout << "Error: File not found.\n";
        return;
    }
    files.get(filename)->snapshot(message);
    updateAnalytics(); // A snapshot might update last modification time.
}

void FileSystem::rollback(const string& filename, int versionId) {
    if (!files.containsKey(filename)) {
        cout << "Error: File not found.\n";
        return;
    }
    if (files.get(filename)->rollback(versionId)) {
        cout << "Rollback successful for '" << filename << "'.\n";
    } else {
        cout << "Error: Rollback failed. Invalid version or already at root.\n";
    }
}

string FileSystem::history(const string& filename) {
    if (!files.containsKey(filename)) return "Error: File not found.\n";
    return files.get(filename)->history();
}

string FileSystem::recentFiles(int num) {
    string result = "";
    int limit = (num == -1) ? files.getValues().size() : num;
    
    result += (num == -1) ? "--- Top All Recently Modified Files ---\n"
                          : "--- Top " + to_string(num) + " Recently Modified Files ---\n";

    // Use a temporary heap to extract max values without destroying the original.
    MaxHeap<FileMetric> tempHeap = recent_files_heap;
    int count = 0;
    while (count < limit && !tempHeap.isEmpty()) {
        FileMetric metric = tempHeap.extractMax();
        time_t t = metric.value;
        char time_buf[100];
        strftime(time_buf, sizeof(time_buf), "%c", localtime(&t));
        result += metric.filename + " (Modified: " + time_buf + ")\n";
        count++;
    }
    return result;
}

string FileSystem::biggestTrees(int num) {
    string result = "";
    int limit = (num == -1) ? files.getValues().size() : num;

    result += (num == -1) ? "--- Top All Files by Version Count ---\n"
                          : "--- Top " + to_string(num) + " Files by Version Count ---\n";
    
    // Use a temporary heap to preserve the original analytics data.
    MaxHeap<FileMetric> tempHeap = biggest_trees_heap;
    int count = 0;
    while (count < limit && !tempHeap.isEmpty()) {
        FileMetric metric = tempHeap.extractMax();
        result += metric.filename + " (" + to_string(metric.value) + " versions)\n";
        count++;
    }
    return result;
}

//==============================================================================
// MAIN FUNCTION & INPUT PARSING
//==============================================================================

/**
 *  A simple, dependency-free parser for command line input.
 *  line The raw input string.
 *  command Output reference for the parsed command.
 *  args Output reference for the vector of arguments.
 */
void parse_input(const string& line, string& command, vector<string>& args) {
    string current_word = "";
    bool first_word = true;

    for (char c : line) {
        if (c == ' ') {
            if (!current_word.empty()) {
                if (first_word) {
                    command = current_word;
                    first_word = false;
                } else {
                    args.push_back(current_word);
                }
                current_word = "";
            }
        } else {
            current_word += c;
        }
    }
    // Add the last word after the loop finishes.
    if (!current_word.empty()) {
        if (first_word) {
            command = current_word;
        } else {
            args.push_back(current_word);
        }
    }
}

/**
 *  The main entry point and command processing loop for the program.
 */
int main() {
    FileSystem anuj;
    cout << "--- Time-Travelling File System ---" << endl;
    cout << "Enter 'QUIT' or 'EXIT' to terminate." << endl;
    string line;
    
    // Main command loop.
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break; // Handle end-of-file (Ctrl+D).
        if (line.empty()) continue;     // Ignore empty lines.

        string command;
        vector<string> args;
        parse_input(line, command, args);

        // --- Command Processing Logic ---

        if (command == "CREATE" && args.size() == 1) {
            anuj.create(args[0]);
        } else if (command == "READ" && args.size() == 1) {
            cout << anuj.read(args[0]) << endl;
        } 
        // Handle commands that can take multi-word content.
        else if ((command == "INSERT" || command == "UPDATE" || command == "SNAPSHOT") && args.size() >= 2) {
            string filename = args[0];
            string message = "";
            // Reconstruct the multi-word content/message from the arguments.
            for (size_t i = 1; i < args.size(); ++i) {
                message += args[i];
                if (i < args.size() - 1) {
                    message += " ";
                }
            }
            if (command == "INSERT") anuj.insert(filename, message);
            if (command == "UPDATE") anuj.update(filename, message);
            if (command == "SNAPSHOT") anuj.snapshot(filename, message);
        } 
        // Handle ROLLBACK with an optional version ID.
        else if (command == "ROLLBACK" && args.size() >= 1 && args.size() <= 2) {
            if (args.size() == 1) {
                anuj.rollback(args[0]); // Rollback to parent.
            } else {
                try {
                    int versionId = stoi(args[1]); // Convert argument to integer.
                    anuj.rollback(args[0], versionId);
                } catch (const invalid_argument& e) {
                    cout << "Error: Invalid version ID for ROLLBACK.\n";
                }
            }
        } else if (command == "HISTORY" && args.size() == 1) {
            cout << anuj.history(args[0]);
        } 
        // Handle analytics commands with an optional number.
        else if (command == "RECENT_FILES" || command == "BIGGEST_TREES") {
            int num = -1; // Default to showing all files.
            if (!args.empty()) {
                try {
                    num = stoi(args[0]);
                } catch (const invalid_argument& e) {
                    cout << "Error: Invalid number. Showing all by default.\n";
                }
            }
            if (command == "RECENT_FILES") cout << anuj.recentFiles(num);
            if (command == "BIGGEST_TREES") cout << anuj.biggestTrees(num);
        } else if (command == "EXIT" || command == "QUIT") {
            cout << "Exiting system." << endl;
            break;
        } else {
            cout << "Error: Unknown command or incorrect arguments.\n";
        }
    }
    return 0;
}