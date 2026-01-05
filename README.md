# Cpp-Version Control System

This project is a simplified, in-memory version control system (anuj) inspired by Git, implemented in C++. It manages files and their version histories using custom-built data structures. The system allows users to create files, track their history through a version tree, and perform operations like creating snapshots, rolling back to previous versions, and viewing system-wide analytics.

---

## Key Features 📝

* **File Management**: Create, read, and modify files within the in-memory file system.
* **Versioning**: Automatically creates new versions of a file when content is added or updated after a snapshot has been made.
* **Immutable Snapshots**: Mark specific versions as "snapshots" to make them immutable, preserving a file's state at a particular point in time.
* **Branching History**: The version history is stored as a tree, naturally supporting branching workflows where different modification paths can exist from a single parent version.
* **Rollback Capability**: Easily navigate a file's history by rolling back to its parent version or any specific snapshot ID.
* **System Analytics**: Use custom-built max-heaps to instantly retrieve analytics, such as the most recently modified files or files with the largest version histories.

---

## Data Structures Implemented 🏗️

As per the assignment requirements, all core data structures were implemented from scratch without using the C++ Standard Library containers.

* **Tree (`VersionNode` struct)**
    * The version history for each file is represented by a tree. Each `VersionNode` stores the file's content, a commit message, timestamps, and pointers to its `parent` and `children`, forming a version graph.

* **HashMap (`HashMap` class)**
    * A custom hash map is used within each `File` object to provide fast, average-time $O(1)$ lookups for any `VersionNode` using its unique integer ID. This is crucial for the `ROLLBACK <filename> <versionID>` operation.

* **Max Heap (`MaxHeap` class)**
    * Two max-heaps are used by the `FileSystem` to track system-wide analytics. One heap organizes files by their last modification time for the `RECENT_FILES` command, and the other organizes them by the total number of versions for the `BIGGEST_TREES` command.

---

## How to Compile and Run ⚙️

The project includes a shell script to simplify compilation.

1.  **Give Execution Permission**: First, make the compile script executable.
    ```bash
    chmod +x compile.sh
    ```
2.  **Compile the Code**: Run the script to compile the `MainCode.cpp` file. This will generate an executable named `anuj`.
    ```bash
    ./compile.sh
    ```
3.  **Run the Program**: Execute the compiled program to start the file system interface.
    ```bash
    ./anuj
    ```

---

## Command Reference ⌨️

The program accepts commands from standard input. Content and messages containing spaces are supported.

### Core File Operations

* **`CREATE <filename>`**
    * Creates a new file with the specified name. The file starts with an initial root version (ID 0) which is also its first snapshot.
    * Example: `CREATE my_document.txt`

* **`READ <filename>`**
    * Displays the content of the file's currently active version.
    * Example: `READ my_document.txt`

* **`INSERT <filename> <content>`**
    * Appends the given content to the file. If the currently active version is a snapshot, this action creates a new child version. Otherwise, it modifies the active version in place.
    * Example: `INSERT my_document.txt This is the first line.`

* **`UPDATE <filename> <content>`**
    * Replaces the file's entire content with the new content. The versioning logic is identical to `INSERT`.
    * Example: `UPDATE my_document.txt This is brand new content.`

* **`SNAPSHOT <filename> <message>`**
    * Marks the currently active version as a snapshot, making its content immutable. It attaches the provided message and a timestamp. You cannot create a snapshot of a version that is already a snapshot.
    * Example: `SNAPSHOT my_document.txt First stable version`

* **`ROLLBACK <filename> [versionID]`**
    * Changes the active version.
    * **Without `versionID`**: Rolls back to the parent of the current version.
    * **With `versionID`**: Switches the active version to the one specified by the ID.
    * Examples:
        ```bash
        ROLLBACK my_document.txt
        ROLLBACK my_document.txt 3
        ```

* **`HISTORY <filename>`**
    * Lists all snapshotted versions on the direct path from the active version back to the root, showing each version's ID, timestamp, and message in chronological order.
    * Example: `HISTORY my_document.txt`

### System-Wide Analytics

* **`RECENT_FILES [num]`**
    * Lists files in descending order of their last modification time. If `num` is provided, the list is limited to the top `num` files. If `num` is omitted, all files are shown.
    * Examples:
        ```bash
        RECENT_FILES 3
        RECENT_FILES
        ```

* **`BIGGEST_TREES [num]`**
    * Lists files in descending order of their total version count. If `num` is provided, it shows the top `num` files. If `num` is omitted, all files are shown.
    * Examples:
        ```bash
        BIGGEST_TREES 5
        BIGGEST_TREES
        ```

### Program Control

* **`EXIT` or `QUIT`**
    * Terminates the program.

---

## Edge Cases and Error Handling ⚠️

The system is designed to handle a variety of edge cases and invalid inputs gracefully by providing clear, descriptive error messages.

### Command and Argument Errors
* **Unknown Command or Incorrect Argument Count**: If a command is misspelled, does not exist, or is provided with the wrong number of arguments, the system will respond with:
    > `Error: Unknown command or incorrect arguments.`
* **Invalid Numeric Input**: For commands that expect a number (like `ROLLBACK`, `RECENT_FILES`, `BIGGEST_TREES`), providing a non-integer value will result in a specific error.
    > `Error: Invalid version ID for ROLLBACK.`
    > `Error: Invalid number. Showing all by default.`
* **Empty Input**: Pressing Enter on a blank line is ignored, and the command prompt is re-displayed.

### File System Errors
* **File Not Found**: Attempting any operation (e.g., `READ`, `INSERT`, `SNAPSHOT`) on a filename that does not exist will result in:
    > `Error: File not found.`
* **File Already Exists**: Attempting to `CREATE` a file with a name that is already in use will fail with the message:
    > `Error: File '<filename>' already exists.`

### Versioning Errors
* **Redundant Snapshot**: You cannot create a snapshot of a version that is already a snapshot. Attempting to do so will produce an error:
    > `Error: A snapshot already exists for the current version. Modify the file to create a new version before snapshotting.`
* **Invalid Rollback Target**: Several rollback scenarios are handled:
    * **Non-existent Version ID**: If you try to roll back to a version ID that does not exist in the file's history, the operation fails:
        > `Error: Rollback failed. Invalid version or already at root.`
    * **Rollback to Active Version**: The system prevents a pointless rollback to the version that is already active:
        > `Error: Cannot rollback to the version that is already active.`
    * **Rollback from Root**: If you are at the root version (ID 0) and attempt to roll back to a parent (which doesn't exist), the operation fails:
        > `Error: Rollback failed. Invalid version or already at root.`

---

## Project Structure 📁

The submission contains the following files:

* `MainCode.cpp`: The complete C++ source code containing all class and function implementations.
* `compile.sh`: A shell script for easy compilation of the project.
* `README.md`: This file.
