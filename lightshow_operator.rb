require 'rubygems'
require 'mqtt'
require 'json'
require 'csv'

MQTT_TOPIC = "example/topic"
UNUSED_LED_PARAMS_STRING = "0,0,0,0,0,-1"
SUPPLIER_RECOMMENDED_POST_DOWNLOAD_DELAY = 15
MQTT_DOWNLOAD_DELAY = 60

class String
  def is_integer?
    self.to_i.to_s == self
  end

  def to_b
    self.casecmp?("t") ? true : false 
  end

  def to_i_with_range(lower_bound, upper_bound = Float::INFINITY)
    raise "wrong bounds" if upper_bound <= lower_bound

    if (lower_bound..upper_bound).include? self.to_i
      self.to_i
    elsif self.to_i < lower_bound
      lower_bound
    elsif self.to_i > upper_bound  
      upper_bound
    end
  end
end

def get_total_sequences_array(sequences_arrays)
  unused_led_params_array = UNUSED_LED_PARAMS_STRING.split(",")

   sequences_arrays.map do |sequence_string_array|
      sequence_params_array = sequence_string_array[0..-2].map {|string| string.split(/,/)}
      number_of_repetitions = sequence_string_array[-1].to_i

      on_or_off_sequence_arrays = sequence_params_array.each_slice(sequence_params_array.size / 2).to_a 
      on_or_off_sequence_arrays * number_of_repetitions
    end.flatten(1).reject {|on_or_off_sequence_arrays| on_or_off_sequence_arrays.all?{|sequence_array| sequence_array == unused_led_params_array}}
end

def to_supplier_defined_csv(sequences_arrays) 
  set_up_first_row = false
  set_up_second_row = false

  CSV.open("geely_lightshow.csv", "w") do |csv|
    3.times do 
      if set_up_first_row and set_up_second_row
        get_total_sequences_array(sequences_arrays).each_with_index do |on_or_off_sequence_arrays, step| 
          led_index_to_sequence_array_hash = on_or_off_sequence_arrays.inject({}) do |hash, sequence_array| 
            hash[sequence_array[5].to_i] = sequence_array
            hash
          end

          full_sequence_signals_array = 48.times.map do |index|
            led_index_to_sequence_array_hash.delete(-1)
            led_index = index + 1

            led_index_to_sequence_array_hash[led_index] ? [step % 8].concat(led_index_to_sequence_array_hash[led_index][0..-2]) : 
                                            [step % 8].concat("0,0,0,0,#{led_index_to_sequence_array_hash.first[1][4]}".split(/,/))
          end

          csv << ["Value #{step + 1}"].concat(full_sequence_signals_array[0..23].zip(full_sequence_signals_array[24..47]).flatten)
        end
      elsif not set_up_first_row
        csv_row = ["Port"]

        left_group_labels = 24.times.map do |index|
          "Group#{index + 1}L"
        end

        right_group_labels = 24.times.map do |index|
          "Group#{index + 1}R"
        end

        left_group_labels.zip(right_group_labels).flatten.each do |label|
          csv_row.concat [label,"","","","",""]
        end

        set_up_first_row = true
        csv << csv_row
      elsif not set_up_second_row
        csv_row = ["Step"]

        48.times.each do |index|
          csv_row.concat ["TimeStamp","mode","LowBriPrm","UpperBriPrm","OffsTiPm","ConTiPrm"]
        end

        set_up_second_row = true
        csv << csv_row
      end
    end

    csv
  end
end

def download(sequences_arrays)
  lightshow_span_in_ms = 0
  MQTT::Client.connect(host: '121.37.165.94',
                       port: 1883,
                       username: 'geely',
                       password:'geely'
                      ) do |client|
    activate_transmission_message = JSON.generate({
      operation: "activation"
    })
    deactivate_transmission_message = JSON.generate({
      operation: "deactivation"
    })

    client.publish(MQTT_TOPIC, activate_transmission_message)

    number_of_frames = 0
    get_total_sequences_array(sequences_arrays).each_with_index do |on_or_off_sequence_arrays, step| 
      leds_signals = convert_sequence_params_array_to_can_signals(on_or_off_sequence_arrays, step % 8)

      transmission_message = JSON.generate({
        operation: "transmission",
        leds_signals: leds_signals
      })
      
      client.publish(MQTT_TOPIC, transmission_message)

      lightshow_span_in_ms += on_or_off_sequence_arrays[0][4].to_i * 10
      number_of_frames += 1
    end

    puts "一共#{number_of_frames}帧"
    
    sleep MQTT_DOWNLOAD_DELAY

    client.publish(MQTT_TOPIC, deactivate_transmission_message)

    sleep SUPPLIER_RECOMMENDED_POST_DOWNLOAD_DELAY
  end
  lightshow_span_in_ms
end

def generate_to_be_downloaded_sequence_strings(one_sided_on_params_array, generate_opposite_side = true, one_by_one = false, left_right_sync = true, 
                                               criss_cross = true, rolling = false, continuous = false, number_of_individual_repetitions = 1, number_of_repetitions = 3)
  number_of_leds = one_sided_on_params_array.size
  continuous_mode_beat_gap_frame = ""

  if continuous
    continuous_mode_beat_gap_frame = one_sided_on_params_array[-1]
    one_sided_on_params_array = one_sided_on_params_array[0..-2]
  end

  if generate_opposite_side
    opposite_sided_on_params_array = rolling ? one_sided_on_params_array.reverse : one_sided_on_params_array
    opposite_sided_on_params_array = opposite_sided_on_params_array.map do |on_params_string| 
      on_array = on_params_string.split(/,/)
      on_array[5] = on_array[5].to_i < 24 ? (on_array[5].to_i + 24).to_s : (on_array[5].to_i - 24).to_s
      on_array.join(",")
    end

    two_sided_on_params_array = one_sided_on_params_array + opposite_sided_on_params_array 
    criss_cross_on_params_array = one_sided_on_params_array.zip(opposite_sided_on_params_array).flatten

    two_sided_off_params_array = two_sided_on_params_array.map.with_index do |on_params_string, index|
      if not continuous
        on_array = on_params_string.split(/,/)
        "0,0,0,0,#{on_array[4]},#{on_array[5]}"
      elsif not rolling and (continuous and index == two_sided_on_params_array.size - 1)
        continuous_mode_beat_gap_frame
      else
        UNUSED_LED_PARAMS_STRING
      end
    end
    criss_cross_off_params_array = criss_cross_on_params_array.map.with_index do |on_params_string, index|
      if not continuous
        on_array = on_params_string.split(/,/)
        "0,0,0,0,#{on_array[4]},#{on_array[5]}"
      elsif continuous and index == criss_cross_on_params_array.size - 1
        continuous_mode_beat_gap_frame
      else
        UNUSED_LED_PARAMS_STRING
      end
    end

    total_sequences_string = if left_right_sync and one_by_one
      left_right_on_params_arrays = criss_cross_on_params_array.each_slice(2).to_a
      left_right_off_params_arrays = criss_cross_off_params_array.each_slice(2).to_a

      left_right_on_params_arrays.zip(left_right_off_params_arrays).map {|on_off_params_strings| on_off_params_strings.flatten.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "}.join " "
    
    elsif left_right_sync and not one_by_one
      two_sided_on_params_array.map{|s|"\"#{s}\""}.join(" ") + " " + two_sided_off_params_array.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "
    
    elsif not left_right_sync and one_by_one
      if criss_cross
        criss_cross_on_params_array.zip(criss_cross_off_params_array).map {|on_off_params_strings| on_off_params_strings.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "}.join " "
      
      elsif rolling  
        opposite_side_leds_range = continuous ? (0..-2) : (1..-2)
        rolling_on_params_array = two_sided_on_params_array + two_sided_on_params_array[opposite_side_leds_range].reverse
        rolling_off_params_array = two_sided_off_params_array + two_sided_off_params_array[opposite_side_leds_range].reverse

        rolling_on_params_array.zip(rolling_off_params_array).map.with_index do |on_off_params_strings, index| 
          if continuous && index == rolling_on_params_array.size - 1
            on_params_string = on_off_params_strings[0]
            on_off_params_strings = [on_params_string, continuous_mode_beat_gap_frame]
          end

          on_off_params_strings.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "
        end.join " "
      else 
        two_sided_on_params_array.zip(two_sided_off_params_array).map {|on_off_params_strings| on_off_params_strings.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "}.join " "
      end

    elsif not left_right_sync and not one_by_one
      left_right_on_params_arrays = two_sided_on_params_array.each_slice(number_of_leds).to_a
      left_right_off_params_arrays = two_sided_off_params_array.each_slice(number_of_leds).to_a

      left_right_on_params_arrays.zip(left_right_off_params_arrays).map {|on_off_params_strings| on_off_params_strings.flatten.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "}.join " "
    end
  else 
    one_sided_off_params_array = one_sided_on_params_array.map.with_index do |on_params_string, index|
      if not continuous or (continuous and index == one_sided_on_params_array.size - 1) 
        on_array = on_params_string.split(/,/)
        "0,0,0,0,#{on_array[4]},#{on_array[5]}"
      else
        UNUSED_LED_PARAMS_STRING
      end
    end

    if one_by_one
      one_sided_on_params_array.zip(one_sided_off_params_array).map {|on_off_params_strings| on_off_params_strings.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "}.join " "
    else
      one_sided_on_params_array.map{|s|"\"#{s}\""}.join(" ") + " " + one_sided_off_params_array.map{|s|"\"#{s}\""}.join(" ") + " #{number_of_individual_repetitions} "
    end
  end * number_of_repetitions
end

def lightshow(lightshow_span_in_ms)
  MQTT::Client.connect(host: '121.37.165.94',
                       port: 1883,
                       username: 'geely',
                       password:'geely'
                      ) do |client|
    start_lightshow_message = JSON.generate({
      operation: "start_lightshow"
    })

    stop_lightshow_message = JSON.generate({
      operation: "stop_lightshow"
    })

    client.publish(MQTT_TOPIC, start_lightshow_message)
    sleep(lightshow_span_in_ms.to_i / 1000)
    client.publish(MQTT_TOPIC, stop_lightshow_message)
  end
end

def convert_sequence_params_array_to_can_signals(on_or_off_sequence_arrays, step = 0)
  led_index_to_sequence_array_hash = on_or_off_sequence_arrays.inject({}) do |hash, sequence_array| 
    hash[sequence_array[5].to_i] = sequence_array
    hash
  end

  full_sequence_can_signals_array = 48.times.map do |index|
    led_index_to_sequence_array_hash.delete(-1)
    led_index = index + 1

    off_leds_signals_array = "0,0,0,0,#{led_index_to_sequence_array_hash.first[1][4]},#{led_index}".split(/,/)

    led_index_to_sequence_array_hash[led_index] ? params_to_hex(led_index_to_sequence_array_hash[led_index], step) : params_to_hex(off_leds_signals_array, step)
  end
end

def params_to_hex(params_array, step)
    can_signals = []

    can_signals << "0x"+params_array[4].to_i.to_s(16)
    can_signals << "0x"+(params_array[1].to_i | params_array[2].to_i << 4).to_s(16) 
    can_signals << "0x"+(params_array[0].to_i << 2).to_s(16)
    can_signals << "0x"+(params_array[3].to_i << 2).to_s(16)
    can_signals << "0x"+(step.to_i << 5 | 1 << 4).to_s(16) 
    3.times {can_signals << "0x00"}

    can_signals
end

def generate_mode_user_prompt
  error_message = "运行灯光秀灯组信息帧信号生成失败。输入n个参数，每个参数为：至少一个同边灯光（左或右）灯组信息：\"模式, 最低亮度, 最高亮度, 开始时间, 结束时间, 灯组号\""
  unless ARGV[1]
    puts error_message
    return
  end
  one_sided_on_params_array = ARGV[1..-1]
  unless one_sided_on_params_array.all? {|on_params_string| on_params_string.split(/,/).size == 6}
    puts error_message
    return
  end

  puts "双边左右灯闪？（T/F, 默认为T/是）？"
  generate_opposite_side = STDIN.gets.strip
  generate_opposite_side = generate_opposite_side.empty? ? true : generate_opposite_side.to_b
  puts generate_opposite_side ? "是" : "否"

  if one_sided_on_params_array.size == 1
    one_by_one = false
  else
    puts "灯组合一起闪？（T/F, 默认为T/是）？"
    one_by_one = STDIN.gets.strip
    one_by_one = one_by_one.empty? ? false : !one_by_one.to_b
    puts !one_by_one ? "是" : "否"
  end

  if one_by_one
    puts "灯组合非一起闪时连续闪（时长越短连续越快）？（T/F, 默认为T/是）？"
    continuous = STDIN.gets.strip
    continuous = continuous.empty? ? true : continuous.to_b
    puts continuous ? "是" : "否"

    if continuous
      default_beat_gap_time = one_sided_on_params_array[0].split(/,/)[4].to_i

      puts "连续闪时每次所有灯组闪动结束后停顿时间？（1-255，默认为第一灯组信息结束时间：#{default_beat_gap_time}）？"
      beat_gap_time = STDIN.gets.strip
      beat_gap_time = beat_gap_time.empty? ? default_beat_gap_time : beat_gap_time.to_i_with_range(1, 255)
      puts beat_gap_time

      one_sided_on_params_array << "0,0,0,0,#{beat_gap_time},1"
    else
      puts "将按照灯组顺序亮灭节奏式闪动！"
    end
  end

  if generate_opposite_side
    puts "左右边灯同步闪？（T/F, 默认为T/是）？"
    left_right_sync = STDIN.gets.strip
    left_right_sync = left_right_sync.empty? ? true : left_right_sync.to_b
    puts left_right_sync ? "是" : "否"

    if one_by_one and not left_right_sync
      puts "左右边灯异步和灯组合非一起闪时左右交错闪？（T/F, 默认为T/是）？"
      criss_cross = STDIN.gets.strip
      criss_cross = criss_cross.empty? ? true : criss_cross.to_b
      puts criss_cross ? "是" : "否"

      if not criss_cross
        puts "左右边灯异步和灯组合非一起闪时左右滚动闪（注意实际灯组左右位置先后）？（T/F, 默认为T/是）？"
        rolling = STDIN.gets.strip
        rolling = rolling.empty? ? true : rolling.to_b
        puts rolling ? "是" : "否"

        if not rolling
          puts "将每边先后各个灯组先后闪！"                    
        end
      end
    end
  end

  puts "单次灯组闪动次数（默认为1）？"
  number_of_individual_repetitions = STDIN.gets.strip
  number_of_individual_repetitions = number_of_individual_repetitions.empty? ? 1 : number_of_individual_repetitions.to_i_with_range(1)
  puts number_of_individual_repetitions

  puts "总循环次数（默认为3）？"
  number_of_repetitions = STDIN.gets.strip
  number_of_repetitions = number_of_repetitions.empty? ? 3 : number_of_repetitions.to_i_with_range(1)
  puts number_of_repetitions

  [one_sided_on_params_array, generate_opposite_side, one_by_one, left_right_sync, criss_cross, rolling, continuous, number_of_individual_repetitions, number_of_repetitions]
end

case ARGV[0]
  when 'download'
    error_message = "运行HCM下载失败。输入n个参数，每个参数为：1|至少一个灯组信息：\"模式, 最低亮度, 最高亮度, 开始时间, 结束时间, 灯组号\"" + 
                    "和2|以上所有灯组信息动作运作次数，最少为1）。"
    unless ARGV[1] && ARGV[2]
      puts error_message
      return
    end
    sequences_arrays = ARGV[1..-1].slice_after {|argument| argument.split(/,/).size == 1}.to_a
    unless sequences_arrays.all? {|sequence_string_array| sequence_string_array[0..-2].all? {|string| string.split(/,/).size == 6} && 
        sequence_string_array[-1].split(/,/).length == 1 && sequence_string_array[-1].to_i > 0}
      puts error_message
      return
    end
    puts (download sequences_arrays).inspect
  when 'lightshow'
    unless ARGV[1]
      puts "运行灯光秀失败。输入参数：灯光秀时间长度（毫秒）。" 
      return
    end
    lightshow ARGV[1]
  when 'can'
    unless ARGV[1] && ARGV[1].split(/,/).size == 6 
      puts "转换灯光动作灯组信息为CAN信号失败。输入参数：1|灯组信息：\"模式, 最低亮度, 最高亮度, 开始时间, 结束时间, 灯组号\"（或2|步骤值，默认为0）。"
      return 
    end
    puts (convert_sequence_params_array_to_can_signals ARGV[1].split(/,/), ARGV[2].dup).inspect
  when 'generate'
    return unless generate_method_params_array = generate_mode_user_prompt

    puts "ruby lightshow_operator.rb download " + (generate_to_be_downloaded_sequence_strings generate_method_params_array[0], generate_method_params_array[1], 
      generate_method_params_array[2], generate_method_params_array[3], generate_method_params_array[4], generate_method_params_array[5], generate_method_params_array[6], generate_method_params_array[7], generate_method_params_array[8])
  when 'generate_and_download'
    return unless generate_method_params_array = generate_mode_user_prompt

    exec "ruby lightshow_operator.rb download " + (generate_to_be_downloaded_sequence_strings generate_method_params_array[0], generate_method_params_array[1], 
      generate_method_params_array[2], generate_method_params_array[3], generate_method_params_array[4], generate_method_params_array[5], generate_method_params_array[6], generate_method_params_array[7], generate_method_params_array[8])
  when 'get_tempo'
    unless ARGV[1] && ARGV[2]
      puts "节奏测试失败。输入参数：1|歌曲片段开始时间(秒)和2｜歌曲片段结束时间(秒)。" 
      return
    end

    song_section_start_time = ARGV[1].to_f
    song_section_end_time = ARGV[2].to_f
    song_section_length_in_sec = song_section_end_time - song_section_start_time

    output = '########################################'
    backspace = "\b"
    space = " "
    backspace_array = []
    space_array = []  
    output.length.times do
      backspace_array << backspace
      space_array << space
    end    
    remove_output = [backspace_array, space_array, backspace_array].join.to_s

    thread = nil
    loop do
      puts "请输入亮灭周期（毫秒）："
      period = STDIN.gets.strip.to_f
      puts "周期是#{period}毫秒！"
      puts "节奏闪需要#{song_section_length_in_sec * 1000 / period * 2}帧"
      puts "节奏闪每帧灯组结束时间参数为#{(period / 2 / 10).to_i}"
      puts "歌曲片段时常为#{song_section_length_in_sec}秒"

      half_tempo_in_seconds = period / 2 / 1000
      thread.terminate if thread

      thread = Thread.new do
        loop do
          print output
          sleep half_tempo_in_seconds
          print remove_output
          sleep half_tempo_in_seconds
        end
      end 
    end
  when 'to_supplier_defined_csv'
     error_message = "灯光秀灯组信息帧信号生成供应商所定格式CSV失败。输入n个参数，每个参数为：1|至少一个灯组信息：\"模式, 最低亮度, 最高亮度, 开始时间, 结束时间, 灯组号\"" + 
                    "和2|以上所有灯组信息动作运作次数，最少为1）。"
    unless ARGV[1] && ARGV[2]
      puts error_message
      return
    end
    sequences_arrays = ARGV[1..-1].slice_after {|argument| argument.split(/,/).size == 1}.to_a
    unless sequences_arrays.all? {|sequence_string_array| sequence_string_array[0..-2].all? {|string| string.split(/,/).size == 6} && 
        sequence_string_array[-1].split(/,/).length == 1 && sequence_string_array[-1].to_i > 0}
      puts error_message
      return
    end

    to_supplier_defined_csv sequences_arrays
  else
    puts "运行失败。可运行模式：download（运行HCM下载）、lightshow（运行灯光秀）、can（转换灯光秀灯组信息为CAN信号）、generate（生成下载到HCM的灯光秀灯组信息帧信号）、" + 
         "generate_and_download（生成灯光秀灯组信息帧信号并运行下载）、get_tempo（节奏计算器）和to_supplier_defined_csv(把灯光秀灯组信息帧信号生成供应商所定格式CSV)。"
end  
