from datetime import datetime
import vk_api
from vk_api.bot_longpoll import VkBotLongPoll, VkBotEventType
from vk_api.utils import get_random_id

from ctypes import *

def log(msg):
    print(datetime.now().strftime('[%d.%m.%Y %H:%M]'), end = " ")
    print(msg)

try:
    math = WinDLL('mathevaluator')
except WindowsError as err:
    log("Error occured while loading MathEvaluator:")
    print(err)
    exit()

math_result_length = 64

log("MathEvaluator loaded uccessfully")

def retrieve_code():
    print("Enter the Two-Factor Authentication code:", end = " ")
    return input(), True

# Sensitive information is stripped away	

vk_token = '########'

vk_group_id = 000000000
vk_group_name = 'groupname'
vk_mention = '[club{}|@{}]'.format(vk_group_id, vk_group_name)

vk_owner_id = 184536960

vk_session = vk_api.VkApi(token = vk_token, auth_handler = retrieve_code)

'''
try:
    vk_session.auth()
except vk_api.AuthError as err:
    print(err)
    exit()
'''

vk = vk_session.get_api()

try:
    vk.groups.enableOnline(group_id = vk_group_id)
except:
    pass

longpoll = VkBotLongPoll(vk_session, vk_group_id)

log("VK Authorization complete. Listening for events...")

for event in longpoll.listen():
    if event.type == VkBotEventType.MESSAGE_NEW:
        obj = event.obj

        if obj.action and obj.action['type'] == 'chat_invite_user' and obj.action['member_id'] == -vk_group_id:
            log("The bot has been added to chat '{}'".format(obj.peer_id))

        msg = obj.text

        # Mention
        if msg.startswith(vk_mention + ' '):
            msg = msg[(len(vk_mention) + 1):]
            mention = True
        else:
            mention = False

		# The owner of the bot can make it close itself
        if obj.from_id == 000000000:
            if msg == 'exit':
                try:
                    vk.groups.disableOnline(group_id = vk_group_id)
                except:
                    pass

                vk.messages.send(
                    peer_id = obj.peer_id,
                    random_id = get_random_id(),
                    message = "The bot is stopped."
                )

                log("Exiting...")
                exit()

        if mention or msg.startswith('eval '):
            if (not mention):
                expression = msg[5:]
            else:
                expression = msg

            result = create_string_buffer(math_result_length)
            try:
                if not math.evaluate_state(expression.encode(), result, math_result_length, obj.from_id):
                    result.value = b"Malformed expression."[:math_result_length]
            except:
                result.value = b"An error has occured while trying to evaluate the expression."[:math_result_length]

            vk.messages.send(
                peer_id = obj.peer_id,
                random_id = get_random_id(),
                message = result.value.decode()
            )

            log("User '{}' evaluated a string: \"{}\" -> \"{}\"".format(obj.from_id, expression, result.value.decode()))
