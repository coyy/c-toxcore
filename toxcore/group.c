/* group.c
 *
 * Slightly better groupchats implementation.
 *
 *  Copyright (C) 2014 Tox project All Rights Reserved.
 *
 *  This file is part of Tox.
 *
 *  Tox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Tox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
 
#include "group.h"
#include "util.h"

/* return 1 if the con_number is not valid.
 * return 0 if the con_number is valid.
 */
static uint8_t con_number_not_valid(const Group_Chats *g_c, int con_number)
{
    if ((unsigned int)con_number >= g_c->num_cons)
        return 1;

    if (g_c->cons == NULL)
        return 1;

    if (g_c->cons[con_number].status == GROUPCON_STATUS_NONE)
        return 1;

    return 0;
}


/* Set the size of the groupchat connections list to num.
 *
 *  return -1 if realloc fails.
 *  return 0 if it succeeds.
 */
static int realloc_groupcons(Group_Chats *g_c, uint32_t num)
{
    if (num == 0) {
        free(g_c->cons);
        g_c->cons = NULL;
        return 0;
    }

    Group_Connection *newgroup_cons = realloc(g_c->cons, num * sizeof(Group_Connection));

    if (newgroup_cons == NULL)
        return -1;

    g_c->cons = newgroup_cons;
    return 0;
}

/* Create a new empty groupchat connection.
 *
 * return -1 on failure.
 * return con_number on success.
 */
static int create_group_con(Group_Chats *g_c)
{
    uint32_t i;

    for (i = 0; i < g_c->num_cons; ++i) {
        if (g_c->cons[i].status == GROUPCON_STATUS_NONE)
            return i;
    }

    int id = -1;

    if (realloc_groupcons(g_c, g_c->num_cons + 1) == 0) {
        id = g_c->num_cons;
        ++g_c->num_cons;
        memset(&(g_c->cons[id]), 0, sizeof(Group_Connection));
    }

    return id;
}

/* Wipe a groupchat connection.
 *
 * return -1 on failure.
 * return 0 on success.
 */
static int wipe_group_con(Group_Chats *g_c, int con_number)
{
    if (con_number_not_valid(g_c, con_number))
        return -1;

    uint32_t i;
    memset(&(g_c->cons[con_number]), 0 , sizeof(Group_c));

    for (i = g_c->num_cons; i != 0; --i) {
        if (g_c->cons[i - 1].status != GROUPCON_STATUS_NONE)
            break;
    }

    if (g_c->num_cons != i) {
        g_c->num_cons = i;
        realloc_groupcons(g_c, g_c->num_cons);
    }

    return 0;
}

static Group_Connection *get_con_group(Group_Chats *g_c, int con_number)
{
    if (con_number_not_valid(g_c, con_number))
        return 0;

    return &g_c->cons[con_number];
}

/* return 1 if the groupnumber is not valid.
 * return 0 if the groupnumber is valid.
 */
static uint8_t groupnumber_not_valid(const Group_Chats *g_c, int groupnumber)
{
    if ((unsigned int)groupnumber >= g_c->num_chats)
        return 1;

    if (g_c->chats == NULL)
        return 1;

    if (g_c->chats[groupnumber].status == GROUPCHAT_STATUS_NONE)
        return 1;

    return 0;
}


/* Set the size of the groupchat list to num.
 *
 *  return -1 if realloc fails.
 *  return 0 if it succeeds.
 */
static int realloc_groupchats(Group_Chats *g_c, uint32_t num)
{
    if (num == 0) {
        free(g_c->chats);
        g_c->chats = NULL;
        return 0;
    }

    Group_c *newgroup_chats = realloc(g_c->chats, num * sizeof(Group_c));

    if (newgroup_chats == NULL)
        return -1;

    g_c->chats = newgroup_chats;
    return 0;
}


/* Create a new empty groupchat connection.
 *
 * return -1 on failure.
 * return groupnumber on success.
 */
static int create_group_chat(Group_Chats *g_c)
{
    uint32_t i;

    for (i = 0; i < g_c->num_chats; ++i) {
        if (g_c->chats[i].status == GROUPCHAT_STATUS_NONE)
            return i;
    }

    int id = -1;

    if (realloc_groupchats(g_c, g_c->num_chats + 1) == 0) {
        id = g_c->num_chats;
        ++g_c->num_chats;
        memset(&(g_c->chats[id]), 0, sizeof(Group_c));
    }

    return id;
}


/* Wipe a groupchat.
 *
 * return -1 on failure.
 * return 0 on success.
 */
static int wipe_group_chat(Group_Chats *g_c, int groupnumber)
{
    if (groupnumber_not_valid(g_c, groupnumber))
        return -1;

    uint32_t i;
    memset(&(g_c->chats[groupnumber]), 0 , sizeof(Group_c));

    for (i = g_c->num_chats; i != 0; --i) {
        if (g_c->chats[i - 1].status != GROUPCHAT_STATUS_NONE)
            break;
    }

    if (g_c->num_chats != i) {
        g_c->num_chats = i;
        realloc_groupchats(g_c, g_c->num_chats);
    }

    return 0;
}

static Group_c *get_group_c(Group_Chats *g_c, int groupnumber)
{
    if (groupnumber_not_valid(g_c, groupnumber))
        return 0;

    return &g_c->chats[groupnumber];
}

/*
 * check if peer with client_id is in peer array.
 *
 * return peer number if peer is in chat.
 * return -1 if peer is not in chat.
 *
 * TODO: make this more efficient.
 */

static int peer_in_chat(const Group_c *chat, const uint8_t *client_id)
{
    uint32_t i;

    for (i = 0; i < chat->numpeers; ++i)
        if (id_equal(chat->group[i].client_id, client_id))
            return i;

    return -1;
}

/*
 * Add a peer to the group chat.
 *
 * return peernum if success or peer already in chat.
 * return -1 if error.
 */
static int addpeer(Group_c *chat, const uint8_t *client_id)
{
    int peernum = peer_in_chat(chat, client_id);

    if (peernum != -1)
        return peernum;

    Group_Peer *temp;
    temp = realloc(chat->group, sizeof(Group_Peer) * (chat->numpeers + 1));

    if (temp == NULL)
        return -1;

    memset(&(temp[chat->numpeers]), 0, sizeof(Group_Peer));
    chat->group = temp;

    id_copy(chat->group[chat->numpeers].client_id, client_id);
    chat->group[chat->numpeers].last_recv = unix_time();
    chat->group[chat->numpeers].last_recv_msgping = unix_time();
    ++chat->numpeers;

    //if (chat->peer_namelistchange != NULL)
    //    (*chat->peer_namelistchange)(chat, chat->numpeers - 1, CHAT_CHANGE_PEER_ADD, chat->group_namelistchange_userdata);

    return (chat->numpeers - 1);
}


/* Creates a new groupchat and puts it in the chats array.
 *
 * return group number on success.
 * return -1 on failure.
 */
int temp_c_add_groupchat(Group_Chats *g_c)
{
    int groupnumber = create_group_chat(g_c);

    Group_c *g = get_group_c(g_c, groupnumber);

    if (!g) {
        return -1;
    }

    g->status = GROUPCHAT_STATUS_VALID;
    return groupnumber;
}

/* Create new groupchat instance. */
Group_Chats *new_groupchats(Messenger *m)
{
    if (!m)
        return NULL;

    Group_Chats *temp = calloc(1, sizeof(Group_Chats));

    if (temp == NULL)
        return NULL;

    temp->m = m;
    return temp;
}

/* main groupchats loop. */
void do_groupchats(Group_Chats *g_c)
{
    //TODO
}

/* Free everything related with group chats. */
void kill_groupchats(Group_Chats *g_c)
{
    //TODO
    free(g_c);
}
