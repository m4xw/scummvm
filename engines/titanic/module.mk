MODULE := engines/titanic

MODULE_OBJS := \
	compressed_file.o \
	compression.o \
	detection.o \
	direct_draw.o \
	font.o \
	game_manager.o \
	game_view.o \
	image.o \
	main_game_window.o \
	screen_manager.o \
	simple_file.o \
	string.o \
	titanic.o \
	video_surface.o \
	core/background.o \
	core/dont_save_file_item.o \
	core/file_item.o \
	core/game_object.o \
	core/link_item.o \
	core/list.o \
	core/message_target.o \
	core/movie_clip.o \
	core/named_item.o \
	core/node_item.o \
	core/pet_control.o \
	core/project_item.o \
	core/resource_key.o \
	core/saveable_object.o \
	core/turn_on_object.o \
	core/turn_on_play_sound.o \
	core/tree_item.o \
	core/view_item.o \
	game/announce.o \
	game/cdrom.o \
	game/cdrom_computer.o \
	game/cdrom_tray.o \
	game/computer_screen.o \
	game/dead_area.o \
	game/hammer_dispensor_button.o \
	game/pet_position.o \
	game/room_item.o \
	game/service_elevator_door.o \
	game/start_action.o \
	game/sub_glass.o \
	game/television.o \
	gfx/act_button.o \
	gfx/changes_season_button.o \
	gfx/chev_left_off.o \
	gfx/chev_left_on.o \
	gfx/chev_right_off.o \
	gfx/chev_right_on.o \
	gfx/chev_send_rec_switch.o \
	gfx/chev_switch.o \
	gfx/elevator_button.o \
	gfx/get_from_succ.o \
	gfx/helmet_on_off.o \
	gfx/home_photo.o \
	gfx/icon_nav_action.o \
	gfx/icon_nav_down.o \
	gfx/icon_nav_left.o \
	gfx/icon_nav_right.o \
	gfx/icon_nav_up.o \
	gfx/keybrd_butt.o \
	gfx/move_object_button.o \
	gfx/pet_mode_off.o \
	gfx/pet_mode_on.o \
	gfx/pet_mode_panel.o \
	gfx/send_to_succ.o \
	gfx/slider_button.o \
	gfx/small_chev_left_off.o \
	gfx/small_chev_left_on.o \
	gfx/small_chev_right_off.o \
	gfx/small_chev_right_on.o \
	gfx/status_change_button.o \
	gfx/st_button.o \
	gfx/toggle_switch.o \
	messages/auto_sound_event.o \
	messages/door_auto_sound_event.o \
	moves/enter_bomb_room.o \
	moves/exit_arboretum.o \
	moves/exit_bridge.o \
	moves/exit_state_room.o \
	moves/exit_titania.o \
	moves/move_player_in_parrot_room.o \
	moves/move_player_to_from.o \
	moves/move_player_to.o \
	moves/multi_move.o \
	moves/pan_from_pel.o \
	moves/restaurant_pan_handler.o \
	moves/restricted_move.o \
	moves/trip_down_canal.o \
	npcs/barbot.o \
	npcs/bellbot.o \
	npcs/callbot.o \
	npcs/character.o \
	npcs/deskbot.o \
	npcs/doorbot.o \
	npcs/liftbot.o \
	npcs/maitre_d.o \
	npcs/mobile.o \
	npcs/parrot.o \
	npcs/robot_controller.o \
	npcs/starlings.o \
	npcs/succubus.o \
	npcs/summon_bots.o \
	npcs/titania.o \
	npcs/true_talk_npc.o \
	sound/auto_music_player.o \
	sound/auto_music_player_base.o \
	sound/seasonal_music_player.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

