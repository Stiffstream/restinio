require File.join( File.dirname(__FILE__), 'lib_finder.rb' )

module RestinioPCRE2Find

  def self.find_libs( libs )

    lib_dirs = ENV[ "LIB" ]
    if lib_dirs

      dirs_to_look = lib_dirs.split( ';' ).select{|p| p != "" }

      dir = dirs_to_look.find_index{|d| RestinioLibFinder.all_files_exist( d, libs ) }
    end

    if dir
      libs
    else
      []
    end
  end

  def self.get_libs_vc
    find_libs( [ 'pcre2-8.lib' ] )
  end

  def self.get_libs_linux
    [ 'pcre2-8' ]
  end

  def self.get_libs_names_linux
    [ 'libpcre2-8.a' ]
  end

  def self.get_libs_mingw
    [ 'pcre2-8' ]
  end

  def self.get_libs_names_mingw
    [ 'libpcre2-8.a' ]
  end

  def self.get_libs( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_mingw
        end
    else
      get_libs_linux
    end
  end

  def self.get_libs_names( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_names_mingw
        end
    else
      get_libs_names_linux
    end
  end

  def self.try_to_get_custom_file
    RestinioLibFinder.try_to_get_custom_file( 'PCRE2_PRJ_FILE', 'local-pcre2.rb' )
  end


  def self.has_pcre2( toolset )
    custom_prj = try_to_get_custom_file
    if File.exist?(custom_prj)
      true
    else
      libraries = get_libs_names( toolset )
      if !libraries.empty?
        RestinioLibFinder.check_libs_available( toolset, libraries )
      else
        false
      end
    end
  end


end #module RestinioPCRE2Find
