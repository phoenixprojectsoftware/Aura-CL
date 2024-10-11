#include <chrono>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_set>

#include <discord_rpc.h>

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "discord_integration.h"

using namespace std::literals::string_literals;

namespace discord_integration
{
	namespace
	{
		// From Discord developer dashboard.
		constexpr const char CLIENT_ID[] = "836328170360799284";

		// This seems to be consistent across PCs.
		constexpr const char STEAM_APP_ID[] = "15569587907822878790"; // This app ID will only launch zamnhlmp and not zamnhlmp_dev - so Discord invites will not work for
																	  // zamnhlmp_dev builds of the game!
		
		// BlueNightHawk : Convert Uppercase Map Names to Lowercase. 2021.
		void LowerCase(const char* in, char* out, int size) 
		{
			for (int i = 0; i < size; i++)
			{
				if (!out[i] || out[i] == '\0')
				{
					out[i] = in[i];
					continue;
				}
				out[i] = (char)tolower(in[i]);
			}
		}


		// Maps for which we have thumbnails.
		const std::unordered_set<std::string> maps_with_thumbnails{
			// PLEASE SORT THESE IN ALPHABETICAL ORDER - AND MAKE SURE THAT THE MAP NAMES YOU PUT HERE 
			// ARE EXACTLY AS THEY ARE CAPITALISED IN THE GAME FILES!
			"2Fort"s,
			"Ascension"s,
			"Base"s,
			"Basement"s,
			"Blimpboom"s,
			"Boot_Box"s,
			"Boot_Camp"s,
			"Bounce"s,
			"Bounce_Day"s,
			"City"s,
			"Cloister"s,
			"Compound"s,
			"Contamination"s,
			"Cove"s,
			"Cove_Night"s,
			"Corssfire"s,
			"Crossfire"s,
			"Crossfire_Day"s,
			"CTF_Boot_Camp"s,
			"CTF_Crossfire"s,
			"CTF_Daydream"s,
			"CTF_Duff"s,
			"CTF_Echo"s,
			"CTF_Forsaken"s,
			"CTF_Gauntlet"s,
			"CTF_Infinity"s,
			"CTF_Omen"s,
			"CTF_Outland"s,
			"CTF_Wormhole"s,
			"CZ2"s,
			"Datacore"s,
			"Datacore2"s,
			"Debris"s,
			"Demise"s,
			"Disposal"s,
			"Doublefort"s,
			"Doublecross"s,
			"Dustbowl"s,
			"Dyce"s,
			"Flashback"s,
			"Frenzy"s,
			"Gasworks"s,
			"Guardian"s,
			"Hex"s,
			"Hierarchy"s,
			"Kasino"s,
			"Kasino2"s,
			"KBase"s,
			"Kndyone"s,
			"Labyrinth"s,
			"Lambda_Bunker"s,
			"Leveled"s,
			"Mall"s,
			"Meanie"s,
			"Office"s,
			"Outpost"s,
			"Overpass"s,
			"Park"s,
			"Pool_Party"s,
			"Pyrozone"s,
			"Radiated"s,
			"Rapidcore"s,
			"Repent"s,
			"Rocket_Frenzy"s,
			"Rubble"s,
			"Rust"s,
			"Rustmill"s,
			"Shipment"s,
			"Signal"s,
			"Skirmish"s,
			"Snark_Pit"s,
			"Snark_Pit2"s,
			"StalkX"s,
			"Stalkyard"s,
			"Stalkyard2"s,
			"Subtarnsit"s,
			"Subtransit"s,
			"Team9"s,
			"TheHill"s,
			"Training_Compound"s,
			"Undertow"s,
			"Walk"s,
			"Water_Canal"s,
			"Waypoint"s,
			"Works"s,
			"Xen"s,
			"Xendance"s
		};

		// Custom maps with thumbnails.
		const std::unordered_set<std::string> custom_maps_with_thumbnails{
			"Bonkfire"s
		};

		// Text names of game states
		const std::string STATE_NAMES[] = {
			"In Menus"s,
			"Playing"s,
			"Spectating"s
		};

		// For tracking if we're in-game.
		bool updated_client_data = false;

		// Possible game states.
		enum class game_state
		{
			NOT_PLAYING = 0,
			PLAYING,
			SPECTATING,
		};

		// Class that handles tracking state changes.
		class DiscordState {
		public:
			DiscordState()
				: cur_state(game_state::NOT_PLAYING)
				, gamemode()
				, player_count(0)
				, player_limit(0)
				, seconds_total(0)
				, start_timestamp(0)
				, match_is_on(false)
				, dirty(true)
			{
				update_presence();


			};

			inline void set_game_state(game_state new_game_state)
			{
				if (cur_state != new_game_state)
				{
					cur_state = new_game_state;

					if (new_game_state == game_state::NOT_PLAYING)
					{
						gamemode.clear();
						match_is_on = false;
						seconds_total = 0;
					}
					else
					{
						refresh_player_stats();
					}

					dirty = true;
				}
			}

			inline game_state get_game_state() const
			{
				return cur_state;
			}

			inline void set_gamemode(std::string new_gamemode)
			{
				if (gamemode != new_gamemode)
				{
					gamemode = std::move(new_gamemode);
					dirty = true;
				}
			}

			inline void set_player_count(int new_player_count)
			{
				if (player_count != new_player_count)
				{
					player_count = new_player_count;
					dirty = true;
				}
			}

			inline void set_player_limit(int new_player_limit)
			{
				if (player_limit != new_player_limit)
				{
					player_limit = new_player_limit;
					dirty = true;
				}
			}

			inline void set_match_is_on(bool new_match_is_on)
			{
				if (match_is_on != new_match_is_on)
				{
					match_is_on = new_match_is_on;
					dirty = true;
				}
			}

			inline void set_time_data(int new_seconds_total, int seconds_passed)
			{
				if (seconds_total != new_seconds_total)
				{
					seconds_total = new_seconds_total;
					const auto current_timestamp = std::chrono::duration_cast<std::chrono::seconds>(
						std::chrono::system_clock::now().time_since_epoch()).count();
					start_timestamp = current_timestamp - seconds_passed;
					dirty = true;
				}
			}

			inline void refresh_player_stats()
			{
				set_player_count(get_player_count());
				set_player_limit(gEngfuncs.GetMaxClients());
			}

			inline void update_presence_if_dirty()
			{
				if (dirty)
					update_presence();
			}

		protected:
			void update_presence()
			{
				dirty = false;

				DiscordRichPresence presence{};

				std::string state = STATE_NAMES[static_cast<size_t>(cur_state)];
				if (cur_state == game_state::PLAYING && match_is_on)
					state = "In a Match"s;

				// Default icon.
				presence.largeImageKey = "icon";

				// Declare these outside of the following block, so they are in scope for Discord_UpdatePresence().
				char map_name[64];
				std::string party_id;

				if (cur_state != game_state::NOT_PLAYING)
				{
					// Get the gamemode.
					if (!gamemode.empty())
						state += " | "s + gamemode;

					// Get the server name.
					if (gViewPort->m_szServerName[0])
						presence.details = gViewPort->m_szServerName;

					// Get the map name and icon.
					get_map_name(map_name, ARRAYSIZE(map_name));
					if (map_name[0])
					{
						char newmapname[64];

						LowerCase((const char*)map_name, newmapname, ARRAYSIZE(map_name));
						if (maps_with_thumbnails.find(map_name) != maps_with_thumbnails.cend())
							presence.largeImageKey = newmapname;

						if (custom_maps_with_thumbnails.find(map_name) != custom_maps_with_thumbnails.cend())
							presence.largeImageKey = newmapname;

						presence.largeImageText = map_name;
					}

					// Get the server address.
					const auto address = get_server_address();
					presence.joinSecret = address;

					party_id = address + "_"s + map_name;
					presence.partyId = party_id.c_str();
					presence.partySize = player_count;
					presence.partyMax = player_limit;

					if (seconds_total != 0)
					{
						presence.startTimestamp = start_timestamp;
						presence.endTimestamp = start_timestamp + seconds_total;
					}
				}

				presence.state = state.c_str();

				Discord_UpdatePresence(&presence);
			}

			game_state cur_state;
			std::string gamemode;
			int player_count;
			int player_limit;
			int seconds_total;
			int64_t start_timestamp;
			bool match_is_on;

			// Flag indicating there are some changes not sent to Discord yet.
			bool dirty;
		};

		// Pointer so the constructor doesn't run too early.
		std::unique_ptr<DiscordState> discord_state;

		// Time of the last update.
		double last_update_time;

		void handle_ready(const DiscordUser*)
		{
			gEngfuncs.Con_Printf("Connected to Discord.\n");
		}

		void handle_errored(int error_code, const char* message)
		{
			gEngfuncs.Con_Printf("Discord error (%d): %s\n", error_code, message);
		}

		void handle_disconnected(int error_code, const char* message)
		{
			gEngfuncs.Con_Printf("Disconnected from Discord (%d): %s\n", error_code, message);
		}

		void handle_joinGame(const char* join_secret)
		{
			std::string address(join_secret);
			sanitize_address(address);

			if (address.empty())
				return;

			auto command = "connect "s + address + "\n"s;

			std::unique_ptr<char[]> temp(new char[command.size() + 1]);
			std::memcpy(temp.get(), command.data(), command.size() + 1);
			EngineClientCmd(temp.get());
		}

		void handle_joinRequest(const DiscordUser* request)
		{
			Discord_Respond(request->userId, DISCORD_REPLY_YES);
		}
	}

	void initialize()
	{
		DiscordEventHandlers handlers{};
		handlers.ready = handle_ready;
		handlers.errored = handle_errored;
		handlers.disconnected = handle_disconnected;
		handlers.joinGame = handle_joinGame;
		handlers.joinRequest = handle_joinRequest;
		Discord_Initialize(CLIENT_ID, &handlers, 1, STEAM_APP_ID);

		discord_state = std::make_unique<DiscordState>();

		Discord_RunCallbacks();

		last_update_time = gEngfuncs.GetAbsoluteTime();
	}

	void shutdown()
	{
		discord_state.reset();
		Discord_Shutdown();
	}

	void set_spectating(bool spectating)
	{
		if (spectating)
			discord_state->set_game_state(game_state::SPECTATING);
		else
			discord_state->set_game_state(game_state::PLAYING);
	}

	void set_gamemode(std::string gamemode)
	{
		discord_state->set_gamemode(std::move(gamemode));
	}

	void set_match_is_on(bool match_is_on)
	{
		discord_state->set_match_is_on(match_is_on);
	}

	void set_time_data(int seconds_total, int seconds_passed)
	{
		discord_state->set_time_data(seconds_total, seconds_passed);
	}

	void on_update_client_data()
	{
		updated_client_data = true;
	}

	void on_frame()
	{
		// Check if we're still in-game.
		if (!updated_client_data)
			discord_state->set_game_state(game_state::NOT_PLAYING);
		else if (discord_state->get_game_state() == game_state::NOT_PLAYING)
			// Only set this if we weren't playing, otherwise we might overwrite some other state.
			discord_state->set_game_state(game_state::PLAYING);

		updated_client_data = false;

		// Check player counts for updates every now and then.
		const auto current_time = gEngfuncs.GetAbsoluteTime();
		if (last_update_time < current_time || last_update_time - current_time > 5)
		{
			last_update_time = current_time;
			on_player_count_update();
			discord_state->update_presence_if_dirty();
		}

		Discord_RunCallbacks();
	}

	void on_player_count_update()
	{
		discord_state->refresh_player_stats();
	}
}
