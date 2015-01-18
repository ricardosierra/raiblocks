#include <gtest/gtest.h>
#include <rai/node.hpp>

TEST (pull_synchronization, empty)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    std::vector <std::unique_ptr <rai::block>> blocks;
    rai::pull_synchronization sync ([&blocks] (rai::block const & block_a)
    {
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_TRUE (sync.synchronize (0));
    ASSERT_EQ (0, blocks.size ());
}

TEST (pull_synchronization, one)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.unchecked_put(block2.hash (), block2);
    rai::pull_synchronization sync ([&blocks] (rai::block const & block_a)
    {
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_FALSE (sync.synchronize (block2.hash ()));
    ASSERT_EQ (1, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
}

TEST (pull_synchronization, send_dependencies)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    rai::send_block block3;
    block3.hashables.previous = block2.hash ();
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.unchecked_put (block2.hash (), block2);
    store.unchecked_put (block3.hash (), block3);
    rai::pull_synchronization sync ([&blocks, &store] (rai::block const & block_a)
    {
        store.block_put (block_a.hash (), block_a);
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_FALSE (sync.synchronize (block3.hash ()));
    ASSERT_EQ (2, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
    ASSERT_EQ (block3, *blocks [1]);
}

TEST (pull_synchronization, change_dependencies)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    rai::change_block block3 (0, block2.hash (), 0, 0);
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.unchecked_put (block2.hash (), block2);
    store.unchecked_put (block3.hash (), block3);
    rai::pull_synchronization sync ([&blocks, &store] (rai::block const & block_a)
                                    {
                                        store.block_put (block_a.hash (), block_a);
                                        blocks.push_back (block_a.clone ());
                                    }, store);
    ASSERT_FALSE (sync.synchronize (block3.hash ()));
    ASSERT_EQ (2, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
    ASSERT_EQ (block3, *blocks [1]);
}

TEST (pull_synchronization, open_dependencies)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    rai::open_block block3;
    block3.hashables.source = block2.hash ();
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.unchecked_put (block2.hash (), block2);
    store.unchecked_put (block3.hash (), block3);
    rai::pull_synchronization sync ([&blocks, &store] (rai::block const & block_a)
    {
        store.block_put (block_a.hash (), block_a);
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_FALSE (sync.synchronize (block3.hash ()));
    ASSERT_EQ (2, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
    ASSERT_EQ (block3, *blocks [1]);
}

TEST (pull_synchronization, receive_dependencies)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    rai::open_block block3;
    block3.hashables.source = block2.hash ();
    rai::send_block block4;
    block4.hashables.previous = block2.hash ();
    rai::receive_block block5;
    block5.hashables.previous = block3.hash ();
    block5.hashables.source = block4.hash ();
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.unchecked_put (block2.hash (), block2);
    store.unchecked_put (block3.hash (), block3);
    store.unchecked_put (block4.hash (), block4);
    store.unchecked_put (block5.hash (), block5);
    rai::pull_synchronization sync ([&blocks, &store] (rai::block const & block_a)
    {
        store.block_put (block_a.hash (), block_a);
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_FALSE (sync.synchronize (block5.hash ()));
    ASSERT_EQ (4, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
    ASSERT_EQ (block3, *blocks [1]);
    ASSERT_EQ (block4, *blocks [2]);
    ASSERT_EQ (block5, *blocks [3]);
}

TEST (pull_synchronization, ladder_dependencies)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    rai::open_block block3;
    block3.hashables.source = block2.hash ();
    rai::send_block block4;
    block4.hashables.previous = block3.hash ();
    rai::receive_block block5;
    block5.hashables.previous = block2.hash ();
    block5.hashables.source = block4.hash ();
    rai::send_block block6;
    block6.hashables.previous = block5.hash ();
    rai::receive_block block7;
    block7.hashables.previous = block4.hash ();
    block7.hashables.source = block6.hash ();
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.unchecked_put (block2.hash (), block2);
    store.unchecked_put (block3.hash (), block3);
    store.unchecked_put (block4.hash (), block4);
    store.unchecked_put (block5.hash (), block5);
    store.unchecked_put (block6.hash (), block6);
    store.unchecked_put (block7.hash (), block7);
    rai::pull_synchronization sync ([&blocks, &store] (rai::block const & block_a)
    {
        store.block_put (block_a.hash (), block_a);
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_FALSE (sync.synchronize (block7.hash ()));
    ASSERT_EQ (6, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
    ASSERT_EQ (block3, *blocks [1]);
    ASSERT_EQ (block4, *blocks [2]);
    ASSERT_EQ (block5, *blocks [3]);
    ASSERT_EQ (block6, *blocks [4]);
    ASSERT_EQ (block7, *blocks [5]);
}

TEST (push_synchronization, empty)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    std::vector <std::unique_ptr <rai::block>> blocks;
    rai::push_synchronization sync ([&blocks] (rai::block const & block_a)
    {
        blocks.push_back (block_a.clone ());
    }, store);
    ASSERT_TRUE (sync.synchronize (0));
    ASSERT_EQ (0, blocks.size ());
}

TEST (push_synchronization, one)
{
    leveldb::Status init;
    rai::block_store store (init, rai::block_store_temp);
	ASSERT_TRUE (init.ok ());
    rai::send_block block1;
    block1.hashables.previous = 1;
    rai::send_block block2;
    block2.hashables.previous = block1.hash ();
    std::vector <std::unique_ptr <rai::block>> blocks;
    store.block_put (block1.hash (), block1);
    store.block_put (block2.hash (), block2);
    rai::push_synchronization sync ([&blocks, &store] (rai::block const & block_a)
    {
        store.block_put (block_a.hash (), block_a);
        blocks.push_back (block_a.clone ());
    }, store);
    store.unsynced_put (block2.hash ());
    ASSERT_FALSE (sync.synchronize (block2.hash ()));
    ASSERT_EQ (1, blocks.size ());
    ASSERT_EQ (block2, *blocks [0]);
}